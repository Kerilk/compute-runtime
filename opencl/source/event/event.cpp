/*
 * Copyright (C) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "opencl/source/event/event.h"

#include "shared/source/command_stream/command_stream_receiver.h"
#include "shared/source/command_stream/task_count_helper.h"
#include "shared/source/command_stream/wait_status.h"
#include "shared/source/device/device.h"
#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/helpers/aligned_memory.h"
#include "shared/source/helpers/flush_stamp.h"
#include "shared/source/helpers/get_info.h"
#include "shared/source/helpers/hw_info.h"
#include "shared/source/helpers/mt_helpers.h"
#include "shared/source/helpers/timestamp_packet.h"
#include "shared/source/memory_manager/internal_allocation_storage.h"
#include "shared/source/utilities/perf_counter.h"
#include "shared/source/utilities/range.h"
#include "shared/source/utilities/tag_allocator.h"

#include "opencl/extensions/public/cl_ext_private.h"
#include "opencl/source/api/cl_types.h"
#include "opencl/source/command_queue/command_queue.h"
#include "opencl/source/context/context.h"
#include "opencl/source/event/async_events_handler.h"
#include "opencl/source/event/event_tracker.h"
#include "opencl/source/helpers/get_info_status_mapper.h"
#include "opencl/source/helpers/hardware_commands_helper.h"
#include "opencl/source/helpers/task_information.h"

#include <algorithm>
#include <iostream>

namespace NEO {
Event::Event(
    Context *ctx,
    CommandQueue *cmdQueue,
    cl_command_type cmdType,
    TaskCountType taskLevel,
    TaskCountType taskCount)
    : taskLevel(taskLevel),
      ctx(ctx),
      cmdQueue(cmdQueue),
      cmdType(cmdType),
      taskCount(taskCount) {
    if (NEO::DebugManager.flags.EventsTrackerEnable.get()) {
        EventsTracker::getEventsTracker().notifyCreation(this);
    }
    flushStamp.reset(new FlushStampTracker(true));

    DBG_LOG(EventsDebugEnable, "Event()", this);

    // Event can live longer than command queue that created it,
    // hence command queue refCount must be incremented
    // non-null command queue is only passed when Base Event object is created
    // any other Event types must increment refcount when setting command queue
    if (cmdQueue != nullptr) {
        cmdQueue->incRefInternal();
    }

    if ((this->ctx == nullptr) && (cmdQueue != nullptr)) {
        this->ctx = &cmdQueue->getContext();
        if (cmdQueue->getTimestampPacketContainer()) {
            timestampPacketContainer = std::make_unique<TimestampPacketContainer>();
        }
    }

    if (this->ctx != nullptr) {
        this->ctx->incRefInternal();
    }

    profilingEnabled = !isUserEvent() &&
                       (cmdQueue ? cmdQueue->getCommandQueueProperties() & CL_QUEUE_PROFILING_ENABLE : false);
    profilingCpuPath = ((cmdType == CL_COMMAND_MAP_BUFFER) || (cmdType == CL_COMMAND_MAP_IMAGE)) && profilingEnabled;

    perfCountersEnabled = cmdQueue ? cmdQueue->isPerfCountersEnabled() : false;
}

Event::Event(
    CommandQueue *cmdQueue,
    cl_command_type cmdType,
    TaskCountType taskLevel,
    TaskCountType taskCount)
    : Event(nullptr, cmdQueue, cmdType, taskLevel, taskCount) {
}

Event::~Event() {
    if (NEO::DebugManager.flags.EventsTrackerEnable.get()) {
        EventsTracker::getEventsTracker().notifyDestruction(this);
    }

    DBG_LOG(EventsDebugEnable, "~Event()", this);
    // no commands should be registred
    DEBUG_BREAK_IF(this->cmdToSubmit.load());

    submitCommand(true);

    int32_t lastStatus = executionStatus;
    if (isStatusCompleted(lastStatus) == false) {
        transitionExecutionStatus(-1);
        DEBUG_BREAK_IF(peekHasCallbacks() || peekHasChildEvents());
    }

    // Note from OCL spec:
    //    "All callbacks registered for an event object must be called.
    //     All enqueued callbacks shall be called before the event object is destroyed."
    if (peekHasCallbacks()) {
        executeCallbacks(lastStatus);
    }

    {
        // clean-up submitted command if needed
        std::unique_ptr<Command> submittedCommand(submittedCmd.exchange(nullptr));
    }

    if (cmdQueue != nullptr) {
        {
            TakeOwnershipWrapper<CommandQueue> queueOwnership(*cmdQueue);
            cmdQueue->handlePostCompletionOperations(true);
        }
        if (timeStampNode != nullptr) {
            timeStampNode->returnTag();
        }
        if (perfCounterNode != nullptr) {
            cmdQueue->getPerfCounters()->deleteQuery(perfCounterNode->getQueryHandleRef());
            perfCounterNode->getQueryHandleRef() = {};
            perfCounterNode->returnTag();
        }
        cmdQueue->decRefInternal();
    }

    if (ctx != nullptr) {
        ctx->decRefInternal();
    }

    // in case event did not unblock child events before
    unblockEventsBlockedByThis(executionStatus);
}

cl_int Event::getEventProfilingInfo(cl_profiling_info paramName,
                                    size_t paramValueSize,
                                    void *paramValue,
                                    size_t *paramValueSizeRet) {
    cl_int retVal;
    const void *src = nullptr;
    size_t srcSize = GetInfo::invalidSourceSize;

    // CL_PROFILING_INFO_NOT_AVAILABLE if event refers to the clEnqueueSVMFree command
    if (isUserEvent() != CL_FALSE ||         // or is a user event object.
        !updateStatusAndCheckCompletion() || // if the execution status of the command identified by event is not CL_COMPLETE
        !profilingEnabled)                   // the CL_QUEUE_PROFILING_ENABLE flag is not set for the command-queue,
    {
        return CL_PROFILING_INFO_NOT_AVAILABLE;
    }

    uint64_t timestamp = 0u;

    // if paramValue is NULL, it is ignored
    switch (paramName) {
    case CL_PROFILING_COMMAND_QUEUED:
        timestamp = getProfilingInfoData(queueTimeStamp);
        src = &timestamp;
        srcSize = sizeof(cl_ulong);
        break;

    case CL_PROFILING_COMMAND_SUBMIT:
        timestamp = getProfilingInfoData(submitTimeStamp);
        src = &timestamp;
        srcSize = sizeof(cl_ulong);
        break;

    case CL_PROFILING_COMMAND_START:
        calcProfilingData();
        timestamp = getProfilingInfoData(startTimeStamp);
        src = &timestamp;
        srcSize = sizeof(cl_ulong);
        break;

    case CL_PROFILING_COMMAND_END:
        calcProfilingData();
        timestamp = getProfilingInfoData(endTimeStamp);
        src = &timestamp;
        srcSize = sizeof(cl_ulong);
        break;

    case CL_PROFILING_COMMAND_COMPLETE:
        calcProfilingData();
        timestamp = getProfilingInfoData(completeTimeStamp);
        src = &timestamp;
        srcSize = sizeof(cl_ulong);
        break;

    case CL_PROFILING_COMMAND_PERFCOUNTERS_INTEL:
        if (!perfCountersEnabled) {
            return CL_INVALID_VALUE;
        }

        if (!cmdQueue->getPerfCounters()->getApiReport(perfCounterNode,
                                                       paramValueSize,
                                                       paramValue,
                                                       paramValueSizeRet,
                                                       updateStatusAndCheckCompletion())) {
            return CL_PROFILING_INFO_NOT_AVAILABLE;
        }
        return CL_SUCCESS;
    default:
        return CL_INVALID_VALUE;
    }

    auto getInfoStatus = GetInfo::getInfo(paramValue, paramValueSize, src, srcSize);
    retVal = changeGetInfoStatusToCLResultType(getInfoStatus);
    GetInfo::setParamValueReturnSize(paramValueSizeRet, srcSize, getInfoStatus);

    return retVal;
} // namespace NEO

void Event::setupBcs(aub_stream::EngineType bcsEngineType) {
    DEBUG_BREAK_IF(!EngineHelpers::isBcs(bcsEngineType));
    this->bcsState.engineType = bcsEngineType;
}

TaskCountType Event::peekBcsTaskCountFromCommandQueue() {
    if (bcsState.isValid()) {
        return this->cmdQueue->peekBcsTaskCount(bcsState.engineType);
    } else {
        return 0u;
    }
}

bool Event::isBcsEvent() const {
    return bcsState.isValid() && bcsState.taskCount > 0;
}

aub_stream::EngineType Event::getBcsEngineType() const {
    return bcsState.engineType;
}

TaskCountType Event::getCompletionStamp() const {
    return this->taskCount;
}

void Event::updateCompletionStamp(TaskCountType gpgpuTaskCount, TaskCountType bcsTaskCount, TaskCountType tasklevel, FlushStamp flushStamp) {
    this->taskCount = gpgpuTaskCount;
    this->bcsState.taskCount = bcsTaskCount;
    this->taskLevel = tasklevel;
    this->flushStamp->setStamp(flushStamp);
}

cl_ulong Event::getDelta(cl_ulong startTime,
                         cl_ulong endTime) {

    auto &hwInfo = cmdQueue->getDevice().getHardwareInfo();

    cl_ulong max = maxNBitValue(hwInfo.capabilityTable.kernelTimestampValidBits);
    cl_ulong delta = 0;

    startTime &= max;
    endTime &= max;

    if (startTime > endTime) {
        delta = max - startTime;
        delta += endTime;
    } else {
        delta = endTime - startTime;
    }

    return delta;
}

void Event::setupRelativeProfilingInfo(ProfilingInfo &profilingInfo) {
    UNRECOVERABLE_IF(!cmdQueue);
    auto &device = cmdQueue->getDevice();
    double resolution = device.getDeviceInfo().profilingTimerResolution;
    UNRECOVERABLE_IF(resolution == 0.0);

    if (profilingInfo.cpuTimeInNs > submitTimeStamp.cpuTimeInNs) {
        auto timeDiff = profilingInfo.cpuTimeInNs - submitTimeStamp.cpuTimeInNs;
        auto gpuTicksDiff = static_cast<uint64_t>(timeDiff / resolution);
        profilingInfo.gpuTimeInNs = submitTimeStamp.gpuTimeInNs + timeDiff;
        profilingInfo.gpuTimeStamp = submitTimeStamp.gpuTimeStamp + gpuTicksDiff;
    } else {
        auto timeDiff = submitTimeStamp.cpuTimeInNs - profilingInfo.cpuTimeInNs;
        auto gpuTicksDiff = static_cast<uint64_t>(timeDiff / resolution);
        profilingInfo.gpuTimeInNs = submitTimeStamp.gpuTimeInNs - timeDiff;
        profilingInfo.gpuTimeStamp = submitTimeStamp.gpuTimeStamp - gpuTicksDiff;
    }
}

void Event::setSubmitTimeStamp(const TimeStampData &submitTimeStamp) {
    UNRECOVERABLE_IF(!cmdQueue);
    auto &device = cmdQueue->getDevice();
    auto &gfxCoreHelper = device.getGfxCoreHelper();
    double resolution = device.getDeviceInfo().profilingTimerResolution;
    UNRECOVERABLE_IF(resolution == 0.0);
    this->submitTimeStamp.cpuTimeInNs = submitTimeStamp.cpuTimeinNS;
    this->submitTimeStamp.gpuTimeInNs = gfxCoreHelper.getGpuTimeStampInNS(submitTimeStamp.gpuTimeStamp, resolution);
    this->submitTimeStamp.gpuTimeStamp = submitTimeStamp.gpuTimeStamp;

    setupRelativeProfilingInfo(queueTimeStamp);
}

uint64_t Event::getProfilingInfoData(const ProfilingInfo &profilingInfo) const {
    if (DebugManager.flags.ReturnRawGpuTimestamps.get()) {
        return profilingInfo.gpuTimeStamp;
    }

    if (DebugManager.flags.EnableDeviceBasedTimestamps.get()) {
        return profilingInfo.gpuTimeInNs;
    }
    return profilingInfo.cpuTimeInNs;
}

bool Event::calcProfilingData() {
    if (!dataCalculated && !profilingCpuPath) {
        if (timestampPacketContainer && timestampPacketContainer->peekNodes().size() > 0) {
            const auto timestamps = timestampPacketContainer->peekNodes();

            if (DebugManager.flags.PrintTimestampPacketContents.get()) {

                for (auto i = 0u; i < timestamps.size(); i++) {
                    std::cout << "Timestamp " << i << ", "
                              << "cmd type: " << this->cmdType << ", ";
                    for (auto j = 0u; j < timestamps[i]->getPacketsUsed(); j++) {
                        std::cout << "packet " << j << ": "
                                  << "global start: " << timestamps[i]->getGlobalStartValue(j) << ", "
                                  << "global end: " << timestamps[i]->getGlobalEndValue(j) << ", "
                                  << "context start: " << timestamps[i]->getContextStartValue(j) << ", "
                                  << "context end: " << timestamps[i]->getContextEndValue(j) << ", "
                                  << "global delta: " << timestamps[i]->getGlobalEndValue(j) - timestamps[i]->getGlobalStartValue(j) << ", "
                                  << "context delta: " << timestamps[i]->getContextEndValue(j) - timestamps[i]->getContextStartValue(j) << std::endl;
                    }
                }
            }

            uint64_t globalStartTS = 0u;
            uint64_t globalEndTS = 0u;
            Event::getBoundaryTimestampValues(timestampPacketContainer.get(), globalStartTS, globalEndTS);

            calculateProfilingDataInternal(globalStartTS, globalEndTS, &globalEndTS, globalStartTS);

        } else if (timeStampNode) {
            if (this->cmdQueue->getDevice().getGfxCoreHelper().useOnlyGlobalTimestamps()) {
                calculateProfilingDataInternal(
                    timeStampNode->getGlobalStartValue(0),
                    timeStampNode->getGlobalEndValue(0),
                    &timeStampNode->getGlobalEndRef(),
                    timeStampNode->getGlobalStartValue(0));
            } else {
                calculateProfilingDataInternal(
                    timeStampNode->getContextStartValue(0),
                    timeStampNode->getContextEndValue(0),
                    &timeStampNode->getContextCompleteRef(),
                    timeStampNode->getGlobalStartValue(0));
            }
        }
    }
    return dataCalculated;
}

void Event::calculateProfilingDataInternal(uint64_t contextStartTS, uint64_t contextEndTS, uint64_t *contextCompleteTS, uint64_t globalStartTS) {
    auto &device = this->cmdQueue->getDevice();
    auto &gfxCoreHelper = device.getGfxCoreHelper();
    auto resolution = device.getDeviceInfo().profilingTimerResolution;

    startTimeStamp.gpuTimeStamp = globalStartTS;
    while (startTimeStamp.gpuTimeStamp < submitTimeStamp.gpuTimeStamp) {
        startTimeStamp.gpuTimeStamp += static_cast<uint64_t>(1ULL << gfxCoreHelper.getGlobalTimeStampBits());
    }
    auto gpuTicksDiff = startTimeStamp.gpuTimeStamp - submitTimeStamp.gpuTimeStamp;
    auto timeDiff = static_cast<uint64_t>(gpuTicksDiff * resolution);
    startTimeStamp.cpuTimeInNs = submitTimeStamp.cpuTimeInNs + timeDiff;
    startTimeStamp.gpuTimeInNs = gfxCoreHelper.getGpuTimeStampInNS(startTimeStamp.gpuTimeStamp, resolution);

    // If device enqueue has not updated complete timestamp, assign end timestamp
    uint64_t gpuDuration = 0;
    uint64_t cpuDuration = 0;

    uint64_t gpuCompleteDuration = 0;
    uint64_t cpuCompleteDuration = 0;

    gpuDuration = getDelta(contextStartTS, contextEndTS);
    if (*contextCompleteTS == 0) {
        *contextCompleteTS = contextEndTS;
        gpuCompleteDuration = gpuDuration;
    } else {
        gpuCompleteDuration = getDelta(contextStartTS, *contextCompleteTS);
    }
    cpuDuration = static_cast<uint64_t>(gpuDuration * resolution);
    cpuCompleteDuration = static_cast<uint64_t>(gpuCompleteDuration * resolution);

    endTimeStamp.cpuTimeInNs = startTimeStamp.cpuTimeInNs + cpuDuration;
    endTimeStamp.gpuTimeInNs = startTimeStamp.gpuTimeInNs + cpuDuration;
    endTimeStamp.gpuTimeStamp = startTimeStamp.gpuTimeStamp + gpuDuration;

    completeTimeStamp.cpuTimeInNs = startTimeStamp.cpuTimeInNs + cpuCompleteDuration;
    completeTimeStamp.gpuTimeInNs = startTimeStamp.gpuTimeInNs + cpuCompleteDuration;
    completeTimeStamp.gpuTimeStamp = startTimeStamp.gpuTimeStamp + gpuCompleteDuration;

    if (DebugManager.flags.ReturnRawGpuTimestamps.get()) {
        startTimeStamp.gpuTimeStamp = contextStartTS;
        endTimeStamp.gpuTimeStamp = contextEndTS;
        completeTimeStamp.gpuTimeStamp = *contextCompleteTS;
    }

    dataCalculated = true;
}

void Event::getBoundaryTimestampValues(TimestampPacketContainer *timestampContainer, uint64_t &globalStartTS, uint64_t &globalEndTS) {
    const auto timestamps = timestampContainer->peekNodes();

    globalStartTS = timestamps[0]->getGlobalStartValue(0);
    globalEndTS = timestamps[0]->getGlobalEndValue(0);

    for (const auto &timestamp : timestamps) {
        if (!timestamp->isProfilingCapable()) {
            continue;
        }
        for (auto i = 0u; i < timestamp->getPacketsUsed(); ++i) {
            if (globalStartTS > timestamp->getGlobalStartValue(i)) {
                globalStartTS = timestamp->getGlobalStartValue(i);
            }
            if (globalEndTS < timestamp->getGlobalEndValue(i)) {
                globalEndTS = timestamp->getGlobalEndValue(i);
            }
        }
    }
}

inline WaitStatus Event::wait(bool blocking, bool useQuickKmdSleep) {
    while (this->taskCount == CompletionStamp::notReady) {
        if (blocking == false) {
            return WaitStatus::NotReady;
        }
    }

    Range<CopyEngineState> states{&bcsState, bcsState.isValid() ? 1u : 0u};
    auto waitStatus = WaitStatus::NotReady;
    auto waitedOnTimestamps = cmdQueue->waitForTimestamps(states, waitStatus, this->timestampPacketContainer.get(), nullptr);
    waitStatus = cmdQueue->waitUntilComplete(taskCount.load(), states, flushStamp->peekStamp(), useQuickKmdSleep, true, waitedOnTimestamps);
    if (waitStatus == WaitStatus::GpuHang) {
        return WaitStatus::GpuHang;
    }

    this->gpuStateWaited = true;

    updateExecutionStatus();

    DEBUG_BREAK_IF(this->taskLevel == CompletionStamp::notReady && this->executionStatus >= 0);

    {
        TakeOwnershipWrapper<CommandQueue> queueOwnership(*cmdQueue);

        bool checkQueueCompletionForPostSyncOperations = !(waitedOnTimestamps && !cmdQueue->isOOQEnabled() &&
                                                           (this->timestampPacketContainer->peekNodes() == cmdQueue->getTimestampPacketContainer()->peekNodes()));

        cmdQueue->handlePostCompletionOperations(checkQueueCompletionForPostSyncOperations);
    }

    auto *allocationStorage = cmdQueue->getGpgpuCommandStreamReceiver().getInternalAllocationStorage();
    allocationStorage->cleanAllocationList(this->taskCount, TEMPORARY_ALLOCATION);
    allocationStorage->cleanAllocationList(this->taskCount, DEFERRED_DEALLOCATION);

    return WaitStatus::Ready;
}

void Event::updateExecutionStatus() {
    if (taskLevel == CompletionStamp::notReady) {
        return;
    }

    int32_t statusSnapshot = executionStatus;
    if (isStatusCompleted(statusSnapshot)) {
        executeCallbacks(statusSnapshot);
        return;
    }

    if (peekIsBlocked()) {
        transitionExecutionStatus(CL_QUEUED);
        executeCallbacks(CL_QUEUED);
        return;
    }

    if (statusSnapshot == CL_QUEUED) {
        bool abortBlockedTasks = isStatusCompletedByTermination(statusSnapshot);
        submitCommand(abortBlockedTasks);
        transitionExecutionStatus(CL_SUBMITTED);
        executeCallbacks(CL_SUBMITTED);
        unblockEventsBlockedByThis(CL_SUBMITTED);
        // Note : Intentional fallthrough (no return) to check for CL_COMPLETE
    }

    if ((cmdQueue != nullptr) && this->isCompleted()) {
        transitionExecutionStatus(CL_COMPLETE);
        executeCallbacks(CL_COMPLETE);
        unblockEventsBlockedByThis(CL_COMPLETE);
        auto *allocationStorage = cmdQueue->getGpgpuCommandStreamReceiver().getInternalAllocationStorage();
        allocationStorage->cleanAllocationList(this->taskCount, TEMPORARY_ALLOCATION);
        allocationStorage->cleanAllocationList(this->taskCount, DEFERRED_DEALLOCATION);
        return;
    }

    transitionExecutionStatus(CL_SUBMITTED);
}

void Event::addChild(Event &childEvent) {
    childEvent.parentCount++;
    childEvent.incRefInternal();
    childEventsToNotify.pushRefFrontOne(childEvent);
    DBG_LOG(EventsDebugEnable, "addChild: Parent event:", this, "child:", &childEvent);
    if (DebugManager.flags.TrackParentEvents.get()) {
        childEvent.parentEvents.push_back(this);
    }
    if (executionStatus == CL_COMPLETE) {
        unblockEventsBlockedByThis(CL_COMPLETE);
    }
}

void Event::unblockEventsBlockedByThis(int32_t transitionStatus) {

    int32_t status = transitionStatus;
    (void)status;
    DEBUG_BREAK_IF(!(isStatusCompleted(status) || (peekIsSubmitted(status))));

    TaskCountType taskLevelToPropagate = CompletionStamp::notReady;

    if (isStatusCompletedByTermination(transitionStatus) == false) {
        // if we are event on top of the tree , obtain taskLevel from CSR
        if (taskLevel == CompletionStamp::notReady) {
            this->taskLevel = getTaskLevel(); // NOLINT(clang-analyzer-optin.cplusplus.VirtualCall)
            taskLevelToPropagate = this->taskLevel;
        } else {
            taskLevelToPropagate = taskLevel + 1;
        }
    }

    auto childEventRef = childEventsToNotify.detachNodes();
    while (childEventRef != nullptr) {
        auto childEvent = childEventRef->ref;

        childEvent->unblockEventBy(*this, taskLevelToPropagate, transitionStatus);

        childEvent->decRefInternal();
        auto next = childEventRef->next;
        delete childEventRef;
        childEventRef = next;
    }
}

bool Event::setStatus(cl_int status) {
    int32_t prevStatus = executionStatus;

    DBG_LOG(EventsDebugEnable, "setStatus event", this, " new status", status, "previousStatus", prevStatus);

    if (isStatusCompleted(prevStatus)) {
        return false;
    }

    if (status == prevStatus) {
        return false;
    }

    if (peekIsBlocked() && (isStatusCompletedByTermination(status) == false)) {
        return false;
    }

    if ((status == CL_SUBMITTED) || (isStatusCompleted(status))) {
        bool abortBlockedTasks = isStatusCompletedByTermination(status);
        submitCommand(abortBlockedTasks);
    }

    this->incRefInternal();
    transitionExecutionStatus(status);
    if (isStatusCompleted(status) || (status == CL_SUBMITTED)) {
        unblockEventsBlockedByThis(status);
    }
    executeCallbacks(status);
    this->decRefInternal();
    return true;
}

void Event::transitionExecutionStatus(int32_t newExecutionStatus) const {
    int32_t prevStatus = executionStatus;
    DBG_LOG(EventsDebugEnable, "transitionExecutionStatus event", this, " new status", newExecutionStatus, "previousStatus", prevStatus);

    while (prevStatus > newExecutionStatus) {
        if (NEO::MultiThreadHelpers::atomicCompareExchangeWeakSpin(executionStatus, prevStatus, newExecutionStatus)) {
            break;
        }
    }
    if (NEO::DebugManager.flags.EventsTrackerEnable.get()) {
        EventsTracker::getEventsTracker().notifyTransitionedExecutionStatus();
    }
}

void Event::submitCommand(bool abortTasks) {
    std::unique_ptr<Command> cmdToProcess(cmdToSubmit.exchange(nullptr));
    if (cmdToProcess.get() != nullptr) {
        getCommandQueue()->initializeBcsEngine(getCommandQueue()->isSpecial());
        auto lockCSR = getCommandQueue()->getGpgpuCommandStreamReceiver().obtainUniqueOwnership();

        if (this->isProfilingEnabled()) {
            if (timeStampNode) {
                this->cmdQueue->getGpgpuCommandStreamReceiver().makeResident(*timeStampNode->getBaseGraphicsAllocation());
                cmdToProcess->timestamp = timeStampNode;
            }
            TimeStampData submitTimeStamp{};
            this->cmdQueue->getDevice().getOSTime()->getGpuCpuTime(&submitTimeStamp);
            this->setSubmitTimeStamp(submitTimeStamp);
            if (profilingCpuPath) {
                setStartTimeStamp();
            } else {
            }
            if (perfCountersEnabled && perfCounterNode) {
                this->cmdQueue->getGpgpuCommandStreamReceiver().makeResident(*perfCounterNode->getBaseGraphicsAllocation());
            }
        }

        auto &complStamp = cmdToProcess->submit(taskLevel, abortTasks);
        if (profilingCpuPath && this->isProfilingEnabled()) {
            setEndTimeStamp();
        }

        if (complStamp.taskCount > CompletionStamp::notReady) {
            abortExecutionDueToGpuHang();
            return;
        }

        updateTaskCount(complStamp.taskCount, peekBcsTaskCountFromCommandQueue());
        flushStamp->setStamp(complStamp.flushStamp);
        submittedCmd.exchange(cmdToProcess.release());
    } else if (profilingCpuPath && endTimeStamp.gpuTimeInNs == 0) {
        setEndTimeStamp();
    }
    if (this->taskCount == CompletionStamp::notReady) {
        if (!this->isUserEvent() && this->eventWithoutCommand) {
            if (this->cmdQueue) {
                auto lockCSR = this->getCommandQueue()->getGpgpuCommandStreamReceiver().obtainUniqueOwnership();
                updateTaskCount(this->cmdQueue->getGpgpuCommandStreamReceiver().peekTaskCount(), peekBcsTaskCountFromCommandQueue());
            }
        }
        // make sure that task count is synchronized for events with kernels
        if (!this->eventWithoutCommand && !abortTasks) {
            this->synchronizeTaskCount();
        }
    }
}

cl_int Event::waitForEvents(cl_uint numEvents,
                            const cl_event *eventList) {
    if (numEvents == 0) {
        return CL_SUCCESS;
    }

    // flush all command queues
    for (const cl_event *it = eventList, *end = eventList + numEvents; it != end; ++it) {
        Event *event = castToObjectOrAbort<Event>(*it);
        if (event->cmdQueue) {
            if (event->taskLevel != CompletionStamp::notReady) {
                event->cmdQueue->flush();
            }
        }
    }

    using WorkerListT = StackVec<cl_event, 64>;
    WorkerListT workerList1(eventList, eventList + numEvents);
    WorkerListT workerList2;
    workerList2.reserve(numEvents);

    // pointers to workerLists - for fast swap operations
    WorkerListT *currentlyPendingEvents = &workerList1;
    WorkerListT *pendingEventsLeft = &workerList2;
    WaitStatus eventWaitStatus = WaitStatus::NotReady;

    while (currentlyPendingEvents->size() > 0) {
        for (auto current = currentlyPendingEvents->begin(), end = currentlyPendingEvents->end(); current != end; ++current) {
            Event *event = castToObjectOrAbort<Event>(*current);
            if (event->peekExecutionStatus() < CL_COMPLETE) {
                return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
            }

            eventWaitStatus = event->wait(false, false);
            if (eventWaitStatus == WaitStatus::NotReady) {
                pendingEventsLeft->push_back(event);
            } else if (eventWaitStatus == WaitStatus::GpuHang) {
                setExecutionStatusToAbortedDueToGpuHang(pendingEventsLeft->begin(), pendingEventsLeft->end());
                setExecutionStatusToAbortedDueToGpuHang(current, end);

                return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
            }
        }

        std::swap(currentlyPendingEvents, pendingEventsLeft);
        pendingEventsLeft->clear();
    }

    return CL_SUCCESS;
}

void Event::setCommand(std::unique_ptr<Command> newCmd) {
    UNRECOVERABLE_IF(cmdToSubmit.load());
    cmdToSubmit.exchange(newCmd.release());
    eventWithoutCommand = false;
}

inline void Event::setExecutionStatusToAbortedDueToGpuHang(cl_event *first, cl_event *last) {
    std::for_each(first, last, [](cl_event &e) {
        Event *event = castToObjectOrAbort<Event>(e);
        event->abortExecutionDueToGpuHang();
    });
}

bool Event::isCompleted() {
    if (gpuStateWaited) {
        return true;
    }

    Range<CopyEngineState> states{&bcsState, bcsState.isValid() ? 1u : 0u};

    if (cmdQueue->isCompleted(getCompletionStamp(), states) || this->areTimestampsCompleted()) {
        gpuStateWaited = true;
    }

    return gpuStateWaited;
}

bool Event::isWaitForTimestampsEnabled() const {
    const auto &productHelper = cmdQueue->getDevice().getRootDeviceEnvironment().getHelper<ProductHelper>();
    auto enabled = cmdQueue->isTimestampWaitEnabled();
    enabled &= productHelper.isTimestampWaitSupportedForEvents();
    enabled &= !cmdQueue->getDevice().getRootDeviceEnvironment().isWddmOnLinux();

    switch (DebugManager.flags.EnableTimestampWaitForEvents.get()) {
    case 0:
        enabled = false;
        break;
    case 1:
        enabled = cmdQueue->getGpgpuCommandStreamReceiver().isUpdateTagFromWaitEnabled();
        break;
    case 2:
        enabled = cmdQueue->getGpgpuCommandStreamReceiver().isDirectSubmissionEnabled();
        break;
    case 3:
        enabled = cmdQueue->getGpgpuCommandStreamReceiver().isAnyDirectSubmissionEnabled();
        break;
    case 4:
        enabled = true;
        break;
    }

    return enabled;
}

bool Event::areTimestampsCompleted() {
    if (this->timestampPacketContainer.get()) {
        if (this->isWaitForTimestampsEnabled()) {
            for (const auto &timestamp : this->timestampPacketContainer->peekNodes()) {
                for (uint32_t i = 0; i < timestamp->getPacketsUsed(); i++) {
                    this->cmdQueue->getGpgpuCommandStreamReceiver().downloadAllocation(*timestamp->getBaseGraphicsAllocation()->getGraphicsAllocation(this->cmdQueue->getGpgpuCommandStreamReceiver().getRootDeviceIndex()));
                    if (timestamp->getContextEndValue(i) == 1) {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

TaskCountType Event::getTaskLevel() {
    return taskLevel;
}

inline void Event::unblockEventBy(Event &event, TaskCountType taskLevel, int32_t transitionStatus) {
    int32_t numEventsBlockingThis = --parentCount;
    DEBUG_BREAK_IF(numEventsBlockingThis < 0);

    int32_t blockerStatus = transitionStatus;
    DEBUG_BREAK_IF(!(isStatusCompleted(blockerStatus) || peekIsSubmitted(blockerStatus)));

    if ((numEventsBlockingThis > 0) && (isStatusCompletedByTermination(blockerStatus) == false)) {
        return;
    }
    DBG_LOG(EventsDebugEnable, "Event", this, "is unblocked by", &event);

    if (this->taskLevel == CompletionStamp::notReady) {
        this->taskLevel = std::max(cmdQueue->getGpgpuCommandStreamReceiver().peekTaskLevel(), taskLevel);
    } else {
        this->taskLevel = std::max(this->taskLevel.load(), taskLevel);
    }

    int32_t statusToPropagate = CL_SUBMITTED;
    if (isStatusCompletedByTermination(blockerStatus)) {
        statusToPropagate = blockerStatus;
    }
    setStatus(statusToPropagate);

    // event may be completed after this operation, transtition the state to not block others.
    this->updateExecutionStatus();
}

bool Event::updateStatusAndCheckCompletion() {
    auto currentStatus = updateEventAndReturnCurrentStatus();
    return isStatusCompleted(currentStatus);
}

bool Event::isReadyForSubmission() {
    return taskLevel != CompletionStamp::notReady ? true : false;
}

void Event::addCallback(Callback::ClbFuncT fn, cl_int type, void *data) {
    ECallbackTarget target = translateToCallbackTarget(type);
    if (target == ECallbackTarget::Invalid) {
        DEBUG_BREAK_IF(true);
        return;
    }
    incRefInternal();

    // Note from spec :
    //    "All callbacks registered for an event object must be called.
    //     All enqueued callbacks shall be called before the event object is destroyed."
    // That's why each registered calback increments the internal refcount
    incRefInternal();
    DBG_LOG(EventsDebugEnable, "event", this, "addCallback", "ECallbackTarget", (uint32_t)type);
    callbacks[(uint32_t)target].pushFrontOne(*new Callback(this, fn, type, data));

    // Callback added after event reached its "completed" state
    if (updateStatusAndCheckCompletion()) {
        int32_t status = executionStatus;
        DBG_LOG(EventsDebugEnable, "event", this, "addCallback executing callbacks with status", status);
        executeCallbacks(status);
    }

    if (peekHasCallbacks() && !isUserEvent() && DebugManager.flags.EnableAsyncEventsHandler.get()) {
        ctx->getAsyncEventsHandler().registerEvent(this);
    }

    decRefInternal();
}

void Event::executeCallbacks(int32_t executionStatusIn) {
    int32_t execStatus = executionStatusIn;
    bool terminated = isStatusCompletedByTermination(execStatus);
    ECallbackTarget target;
    if (terminated) {
        target = ECallbackTarget::Completed;
    } else {
        target = translateToCallbackTarget(execStatus);
        if (target == ECallbackTarget::Invalid) {
            DEBUG_BREAK_IF(true);
            return;
        }
    }

    // run through all needed callback targets and execute callbacks
    for (uint32_t i = 0; i <= (uint32_t)target; ++i) {
        auto cb = callbacks[i].detachNodes();
        auto curr = cb;
        while (curr != nullptr) {
            auto next = curr->next;
            if (terminated) {
                curr->overrideCallbackExecutionStatusTarget(execStatus);
            }
            DBG_LOG(EventsDebugEnable, "event", this, "executing callback", "ECallbackTarget", (uint32_t)target);
            curr->execute();
            decRefInternal();
            delete curr;
            curr = next;
        }
    }
}

bool Event::tryFlushEvent() {
    // only if event is not completed, completed event has already been flushed
    if (cmdQueue && updateStatusAndCheckCompletion() == false) {
        // flush the command queue only if it is not blocked event
        if (taskLevel != CompletionStamp::notReady) {
            return cmdQueue->getGpgpuCommandStreamReceiver().flushBatchedSubmissions();
        }
    }
    return true;
}

void Event::setQueueTimeStamp() {
    UNRECOVERABLE_IF(!cmdQueue);
    this->cmdQueue->getDevice().getOSTime()->getCpuTime(&queueTimeStamp.cpuTimeInNs);
}

void Event::setStartTimeStamp() {
    UNRECOVERABLE_IF(!cmdQueue);
    this->cmdQueue->getDevice().getOSTime()->getCpuTime(&startTimeStamp.cpuTimeInNs);
    setupRelativeProfilingInfo(startTimeStamp);
}

void Event::setEndTimeStamp() {
    UNRECOVERABLE_IF(!cmdQueue);
    this->cmdQueue->getDevice().getOSTime()->getCpuTime(&endTimeStamp.cpuTimeInNs);
    setupRelativeProfilingInfo(endTimeStamp);
    completeTimeStamp = endTimeStamp;
}

TagNodeBase *Event::getHwTimeStampNode() {
    if (!cmdQueue->getTimestampPacketContainer() && !timeStampNode) {
        timeStampNode = cmdQueue->getGpgpuCommandStreamReceiver().getEventTsAllocator()->getTag();
    }
    return timeStampNode;
}

TagNodeBase *Event::getHwPerfCounterNode() {
    if (!perfCounterNode && cmdQueue->getPerfCounters()) {
        const uint32_t gpuReportSize = HwPerfCounter::getSize(*(cmdQueue->getPerfCounters()));
        perfCounterNode = cmdQueue->getGpgpuCommandStreamReceiver().getEventPerfCountAllocator(gpuReportSize)->getTag();
    }
    return perfCounterNode;
}

TagNodeBase *Event::getMultiRootTimestampSyncNode() {
    auto lock = getContext()->obtainOwnershipForMultiRootDeviceAllocator();
    if (getContext()->getMultiRootDeviceTimestampPacketAllocator() == nullptr) {
        auto allocator = cmdQueue->getGpgpuCommandStreamReceiver().createMultiRootDeviceTimestampPacketAllocator(getContext()->getRootDeviceIndices());
        getContext()->setMultiRootDeviceTimestampPacketAllocator(allocator);
    }
    lock.unlock();
    if (multiRootDeviceTimestampPacketContainer.get() == nullptr) {
        multiRootDeviceTimestampPacketContainer = std::make_unique<TimestampPacketContainer>();
    }
    multiRootTimeStampSyncNode = getContext()->getMultiRootDeviceTimestampPacketAllocator()->getTag();
    multiRootDeviceTimestampPacketContainer->add(multiRootTimeStampSyncNode);
    return multiRootTimeStampSyncNode;
}

void Event::addTimestampPacketNodes(const TimestampPacketContainer &inputTimestampPacketContainer) {
    timestampPacketContainer->assignAndIncrementNodesRefCounts(inputTimestampPacketContainer);
}

TimestampPacketContainer *Event::getTimestampPacketNodes() const { return timestampPacketContainer.get(); }
TimestampPacketContainer *Event::getMultiRootDeviceTimestampPacketNodes() const { return multiRootDeviceTimestampPacketContainer.get(); }

bool Event::checkUserEventDependencies(cl_uint numEventsInWaitList, const cl_event *eventWaitList) {
    bool userEventsDependencies = false;

    for (uint32_t i = 0; i < numEventsInWaitList; i++) {
        auto event = castToObjectOrAbort<Event>(eventWaitList[i]);
        if (!event->isReadyForSubmission()) {
            userEventsDependencies = true;
            break;
        }
    }
    return userEventsDependencies;
}

TaskCountType Event::peekTaskLevel() const {
    return taskLevel;
}

} // namespace NEO
