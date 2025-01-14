/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/core/test/unit_tests/mocks/mock_cmdlist.h"

namespace L0 {
namespace ult {

WhiteBox<::L0::CommandListImp>::WhiteBox() : BaseClass(BaseClass::defaultNumIddsPerBlock) {}

WhiteBox<::L0::CommandListImp>::~WhiteBox() {}

MockCommandList::MockCommandList(Device *device) : WhiteBox<::L0::CommandListImp>() {
    this->device = device;
    size_t batchBufferSize = 65536u;
    batchBuffer = new uint8_t[batchBufferSize];
    mockAllocation = new NEO::GraphicsAllocation(0,
                                                 NEO::AllocationType::INTERNAL_HOST_MEMORY,
                                                 &batchBuffer,
                                                 reinterpret_cast<uint64_t>(&batchBuffer),
                                                 0,
                                                 sizeof(batchBufferSize),
                                                 NEO::MemoryPool::System4KBPages,
                                                 NEO::MemoryManager::maxOsContextCount);
}

MockCommandList::~MockCommandList() {
    delete mockAllocation;
    delete[] batchBuffer;
}

} // namespace ult
} // namespace L0
