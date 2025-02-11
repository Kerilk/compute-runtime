/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/unit_test/fixtures/front_window_fixture.h"

#include "shared/source/command_container/cmdcontainer.h"
#include "shared/source/gmm_helper/gmm_helper.h"
#include "shared/source/memory_manager/gfx_partition.h"
#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/mocks/mock_device.h"
#include "shared/test/common/test_macros/test.h"

using namespace NEO;

MemManagerFixture::FrontWindowMemManagerMock::FrontWindowMemManagerMock(NEO::ExecutionEnvironment &executionEnvironment) : MockMemoryManager(executionEnvironment) {}
void MemManagerFixture::FrontWindowMemManagerMock::forceLimitedRangeAllocator(uint32_t rootDeviceIndex, uint64_t range) { getGfxPartition(rootDeviceIndex)->init(range, 0, 0, gfxPartitions.size(), true, 0u); }

void MemManagerFixture::setUp() {
    DebugManagerStateRestore dbgRestorer;
    DebugManager.flags.UseExternalAllocatorForSshAndDsh.set(true);
    DeviceFixture::setUp();
    memManager = std::unique_ptr<FrontWindowMemManagerMock>(new FrontWindowMemManagerMock(*pDevice->getExecutionEnvironment()));
}

void MemManagerFixture::tearDown() {
    DeviceFixture::tearDown();
}

BindlessCommandEncodeStatesFixture::~BindlessCommandEncodeStatesFixture() {
}

void BindlessCommandEncodeStatesFixture::setUp() {
    MemManagerFixture::setUp();

    auto &productHelper = pDevice->getProductHelper();
    this->l1CachePolicyData.init(productHelper);

    cmdContainer = std::make_unique<CommandContainer>();
    cmdContainer->l1CachePolicyDataRef() = &l1CachePolicyData;
}

void BindlessCommandEncodeStatesFixture::tearDown() {
    cmdContainer.reset(nullptr);
    MemManagerFixture::tearDown();
}
