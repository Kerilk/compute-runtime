/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "shared/source/command_container/cmdcontainer.h"
#include "shared/source/command_stream/linear_stream.h"
#include "shared/source/command_stream/preemption_mode.h"
#include "shared/source/command_stream/submission_status.h"
#include "shared/source/command_stream/task_count_helper.h"
#include "shared/source/command_stream/wait_status.h"
#include "shared/source/helpers/completion_stamp.h"
#include "shared/source/utilities/stackvec.h"

#include "level_zero/core/source/cmdqueue/cmdqueue.h"

#include <atomic>
#include <vector>

struct UnifiedMemoryControls;

namespace NEO {
class LinearStream;
class GraphicsAllocation;
class MemoryManager;

} // namespace NEO

namespace L0 {
struct CommandList;
struct Kernel;
struct CommandQueueImp : public CommandQueue {
    class CommandBufferManager {
      public:
        enum BUFFER_ALLOCATION : uint32_t {
            FIRST = 0,
            SECOND,
            COUNT
        };

        ze_result_t initialize(Device *device, size_t sizeRequested);
        void destroy(Device *device);
        NEO::WaitStatus switchBuffers(NEO::CommandStreamReceiver *csr);

        NEO::GraphicsAllocation *getCurrentBufferAllocation() {
            return buffers[bufferUse];
        }

        void setCurrentFlushStamp(TaskCountType taskCount, NEO::FlushStamp flushStamp) {
            flushId[bufferUse] = std::make_pair(taskCount, flushStamp);
        }
        std::pair<TaskCountType, NEO::FlushStamp> &getCurrentFlushStamp() {
            return flushId[bufferUse];
        }

      private:
        NEO::GraphicsAllocation *buffers[BUFFER_ALLOCATION::COUNT]{};
        std::pair<TaskCountType, NEO::FlushStamp> flushId[BUFFER_ALLOCATION::COUNT];
        BUFFER_ALLOCATION bufferUse = BUFFER_ALLOCATION::FIRST;
    };
    static constexpr size_t defaultQueueCmdBufferSize = 128 * MemoryConstants::kiloByte;
    static constexpr size_t minCmdBufferPtrAlign = 8;
    static constexpr size_t totalCmdBufferSize =
        defaultQueueCmdBufferSize +
        MemoryConstants::cacheLineSize +
        NEO::CSRequirements::csOverfetchSize;

    CommandQueueImp() = delete;
    CommandQueueImp(Device *device, NEO::CommandStreamReceiver *csr, const ze_command_queue_desc_t *desc);

    ze_result_t destroy() override;

    ze_result_t synchronize(uint64_t timeout) override;

    ze_result_t initialize(bool copyOnly, bool isInternal, bool immediateCmdListQueue);

    Device *getDevice() { return device; }

    NEO::CommandStreamReceiver *getCsr() { return csr; }

    MOCKABLE_VIRTUAL NEO::WaitStatus reserveLinearStreamSize(size_t size);

    ze_command_queue_mode_t getCommandQueueMode() const {
        return desc.mode;
    }

    bool isSynchronousMode() const {
        return getCommandQueueMode() == ZE_COMMAND_QUEUE_MODE_SYNCHRONOUS;
    }

    virtual bool getPreemptionCmdProgramming() = 0;
    void handleIndirectAllocationResidency(UnifiedMemoryControls unifiedMemoryControls, std::unique_lock<std::mutex> &lockForIndirect, bool performMigration) override;
    void makeResidentAndMigrate(bool performMigration, const NEO::ResidencyContainer &residencyContainer) override;
    void printKernelsPrintfOutput(bool hangDetected);
    void checkAssert();
    void unregisterCsrClient() override;
    void registerCsrClient() override;

  protected:
    MOCKABLE_VIRTUAL NEO::SubmissionStatus submitBatchBuffer(size_t offset, NEO::ResidencyContainer &residencyContainer, void *endingCmdPtr,
                                                             bool isCooperative);

    ze_result_t synchronizeByPollingForTaskCount(uint64_t timeout);

    void postSyncOperations(bool hangDetected);

    static constexpr uint32_t defaultCommandListStateChangeListSize = 10;
    struct CommandListDirtyFlags {
        bool propertyScmDirty = false;
        bool propertyFeDirty = false;
        bool propertyPsDirty = false;
        bool propertySbaDirty = false;
        bool frontEndReturnPoint = false;
        bool preemptionDirty = false;

        bool isAnyDirty() const {
            return (propertyScmDirty || propertyFeDirty || propertyPsDirty || propertySbaDirty || frontEndReturnPoint || preemptionDirty);
        }
        void cleanDirty() {
            propertyScmDirty = false;
            propertyFeDirty = false;
            propertyPsDirty = false;
            propertySbaDirty = false;
            frontEndReturnPoint = false;
            preemptionDirty = false;
        }
    };
    struct CommandListRequiredStateChange {
        CommandListRequiredStateChange() = default;
        NEO::StreamProperties requiredState{};
        CommandList *commandList = nullptr;
        CommandListDirtyFlags flags{};
        NEO::PreemptionMode newPreemptionMode = NEO::PreemptionMode::Initial;
        uint32_t cmdListIndex = 0;
    };

    using CommandListStateChangeList = StackVec<CommandListRequiredStateChange, CommandQueueImp::defaultCommandListStateChangeListSize>;

    CommandListStateChangeList stateChanges;
    CommandBufferManager buffers;
    NEO::LinearStream commandStream{};
    NEO::LinearStream firstCmdListStream{};
    NEO::HeapContainer heapContainer;
    ze_command_queue_desc_t desc;
    std::vector<Kernel *> printfKernelContainer;

    Device *device = nullptr;
    NEO::CommandStreamReceiver *csr = nullptr;
    NEO::LinearStream *startingCmdBuffer = nullptr;

    uint32_t currentStateChangeIndex = 0;

    std::atomic<bool> cmdListWithAssertExecuted = false;
    bool useKmdWaitFunction = false;
};

} // namespace L0
