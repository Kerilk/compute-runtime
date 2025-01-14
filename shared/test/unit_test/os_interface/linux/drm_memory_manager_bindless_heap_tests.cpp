/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/source/gmm_helper/gmm_helper.h"
#include "shared/source/helpers/bindless_heaps_helper.h"
#include "shared/source/indirect_heap/indirect_heap.h"
#include "shared/source/os_interface/device_factory.h"
#include "shared/source/os_interface/os_interface.h"
#include "shared/test/common/fixtures/device_fixture.h"
#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/helpers/default_hw_info.h"
#include "shared/test/common/mocks/linux/mock_drm_memory_manager.h"
#include "shared/test/common/mocks/mock_allocation_properties.h"
#include "shared/test/common/os_interface/linux/drm_memory_manager_fixture.h"
#include "shared/test/common/test_macros/hw_test.h"

namespace NEO {
struct GlobalBindlessDrmMemManagerFixture : public DrmMemoryManagerFixtureWithoutQuietIoctlExpectation {
    GlobalBindlessDrmMemManagerFixture() : DrmMemoryManagerFixtureWithoutQuietIoctlExpectation(1, 0) {}
    void setUp() {
        DebugManager.flags.UseExternalAllocatorForSshAndDsh.set(true);
        DrmMemoryManagerFixtureWithoutQuietIoctlExpectation::setUp(true);
    }
    void tearDown() {
    }

    DebugManagerStateRestore dbgRestorer;
};

using DrmGlobalBindlessAllocatorTests = Test<GlobalBindlessDrmMemManagerFixture>;

TEST_F(DrmGlobalBindlessAllocatorTests, givenLocalMemoryWhenSurfaceStatesAllocationCreatedThenGpuBaseAddressIsSetToCorrectBaseAddress) {
    MockAllocationProperties properties(rootDeviceIndex, true, MemoryConstants::pageSize64k, AllocationType::LINEAR_STREAM);
    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->createBindlessHeapsHelper(memoryManager.get(), false, rootDeviceIndex, 1);
    auto allocation = memoryManager->allocateGraphicsMemoryInPreferredPool(properties, nullptr);
    ASSERT_NE(nullptr, allocation);
    auto gmmHelper = memoryManager->getGmmHelper(rootDeviceIndex);
    EXPECT_EQ(gmmHelper->canonize(memoryManager->getExternalHeapBaseAddress(allocation->getRootDeviceIndex(), allocation->isAllocatedInLocalMemoryPool())), allocation->getGpuBaseAddress());

    ASSERT_NE(nullptr, executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper());
    EXPECT_EQ(executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getGlobalHeapsBase(), allocation->getGpuBaseAddress());

    memoryManager->freeGraphicsMemory(allocation);
    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->bindlessHeapsHelper.reset();
}

TEST_F(DrmGlobalBindlessAllocatorTests, givenLocalMemoryWhenSurfaceStatesAllocationCreatedInDevicePoolThenGpuBaseAddressIsSetToBindlessBaseAddress) {
    DebugManager.flags.ForceLocalMemoryAccessMode.set(0);
    AllocationData allocData = {};
    allocData.type = AllocationType::LINEAR_STREAM;
    allocData.size = MemoryConstants::pageSize64k;

    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->createBindlessHeapsHelper(memoryManager.get(), false, rootDeviceIndex, 1);
    MemoryManager::AllocationStatus status;
    auto allocation = memoryManager->allocateGraphicsMemoryInDevicePool(allocData, status);
    ASSERT_NE(nullptr, allocation);
    auto gmmHelper = memoryManager->getGmmHelper(rootDeviceIndex);
    EXPECT_EQ(gmmHelper->canonize(memoryManager->getExternalHeapBaseAddress(allocation->getRootDeviceIndex(), true)), allocation->getGpuBaseAddress());
    ASSERT_NE(nullptr, executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper());
    EXPECT_EQ(executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getGlobalHeapsBase(), allocation->getGpuBaseAddress());
    EXPECT_LT(executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getGlobalHeapsBase(), allocation->getGpuAddress());

    memoryManager->freeGraphicsMemory(allocation);
    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->bindlessHeapsHelper.reset();
}

TEST_F(DrmGlobalBindlessAllocatorTests, givenLocalMemoryWhenSpecialSshHeapCreatedInDevicePoolThenGpuAddressIsSetToBindlessBaseAddress) {
    DebugManager.flags.ForceLocalMemoryAccessMode.set(0);
    AllocationData allocData = {};
    allocData.type = AllocationType::LINEAR_STREAM;
    allocData.size = MemoryConstants::pageSize64k;

    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->createBindlessHeapsHelper(memoryManager.get(), false, rootDeviceIndex, 1);

    auto gmmHelper = memoryManager->getGmmHelper(rootDeviceIndex);
    EXPECT_EQ(gmmHelper->canonize(memoryManager->getExternalHeapBaseAddress(rootDeviceIndex, true)),
              executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getGlobalHeapsBase());

    EXPECT_EQ(executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getGlobalHeapsBase(),
              executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getHeap(BindlessHeapsHelper::BindlesHeapType::SPECIAL_SSH)->getGraphicsAllocation()->getGpuAddress());

    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->bindlessHeapsHelper.reset();
}

TEST_F(DrmGlobalBindlessAllocatorTests, givenLocalMemoryWhenSurfaceStatesAllocationCreatedInPreferredPoolThenGpuBaseAddressIsSetToCorrectBaseAddress) {
    AllocationData allocData = {};
    allocData.type = AllocationType::LINEAR_STREAM;
    allocData.size = MemoryConstants::pageSize64k;

    executionEnvironment->rootDeviceEnvironments[0]->createBindlessHeapsHelper(memoryManager.get(), false, rootDeviceIndex, 1);

    AllocationProperties properties = {rootDeviceIndex, MemoryConstants::pageSize64k, AllocationType::LINEAR_STREAM, {}};
    auto allocation = memoryManager->allocateGraphicsMemoryInPreferredPool(properties, nullptr);
    ASSERT_NE(nullptr, allocation);
    auto gmmHelper = memoryManager->getGmmHelper(rootDeviceIndex);
    bool deviceMemory = allocation->isAllocatedInLocalMemoryPool();
    EXPECT_EQ(gmmHelper->canonize(memoryManager->getExternalHeapBaseAddress(allocation->getRootDeviceIndex(), deviceMemory)), allocation->getGpuBaseAddress());

    ASSERT_NE(nullptr, executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper());
    EXPECT_EQ(executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->getBindlessHeapsHelper()->getGlobalHeapsBase(), allocation->getGpuBaseAddress());

    memoryManager->freeGraphicsMemory(allocation);
    executionEnvironment->rootDeviceEnvironments[rootDeviceIndex]->bindlessHeapsHelper.reset();
}
} // namespace NEO