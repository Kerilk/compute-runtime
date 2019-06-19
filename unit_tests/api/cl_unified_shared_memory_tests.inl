/*
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/api/api.h"
#include "runtime/memory_manager/svm_memory_manager.h"
#include "unit_tests/mocks/mock_context.h"
#include "unit_tests/mocks/mock_kernel.h"

using namespace NEO;

TEST(clUnifiedSharedMemoryTests, whenClHostMemAllocINTELisCalledWithoutContextThenInvalidContextIsReturned) {
    cl_int retVal = CL_SUCCESS;
    auto ptr = clHostMemAllocINTEL(0, nullptr, 0, 0, &retVal);
    EXPECT_EQ(nullptr, ptr);
    EXPECT_EQ(CL_INVALID_CONTEXT, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClHostMemAllocIntelIsCalledThenItAllocatesHostUnifiedMemoryAllocation) {
    MockContext mockContext;
    cl_int retVal = CL_SUCCESS;
    auto unifiedMemoryHostAllocation = clHostMemAllocINTEL(&mockContext, nullptr, 4, 0, &retVal);
    EXPECT_EQ(CL_SUCCESS, retVal);
    ASSERT_NE(nullptr, unifiedMemoryHostAllocation);

    auto allocationsManager = mockContext.getSVMAllocsManager();
    EXPECT_EQ(1u, allocationsManager->getNumAllocs());
    auto graphicsAllocation = allocationsManager->getSVMAlloc(unifiedMemoryHostAllocation);
    EXPECT_EQ(graphicsAllocation->size, 4u);
    EXPECT_EQ(graphicsAllocation->memoryType, InternalMemoryType::HOST_UNIFIED_MEMORY);
    EXPECT_EQ(graphicsAllocation->gpuAllocation->getGpuAddress(), castToUint64(unifiedMemoryHostAllocation));

    retVal = clMemFreeINTEL(&mockContext, unifiedMemoryHostAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClDeviceMemAllocINTELisCalledWithWrongContextThenInvalidContextErrorIsReturned) {
    cl_int retVal = CL_SUCCESS;
    auto ptr = clDeviceMemAllocINTEL(0, 0, nullptr, 0, 0, &retVal);
    EXPECT_EQ(nullptr, ptr);
    EXPECT_EQ(CL_INVALID_CONTEXT, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClDeviceMemAllocIntelIsCalledThenItAllocatesDeviceUnifiedMemoryAllocation) {
    MockContext mockContext;
    cl_int retVal = CL_SUCCESS;
    auto unfiedMemoryDeviceAllocation = clDeviceMemAllocINTEL(&mockContext, mockContext.getDevice(0u), nullptr, 4, 0, &retVal);
    EXPECT_EQ(CL_SUCCESS, retVal);
    ASSERT_NE(nullptr, unfiedMemoryDeviceAllocation);

    auto allocationsManager = mockContext.getSVMAllocsManager();
    EXPECT_EQ(1u, allocationsManager->getNumAllocs());
    auto graphicsAllocation = allocationsManager->getSVMAlloc(unfiedMemoryDeviceAllocation);
    EXPECT_EQ(graphicsAllocation->size, 4u);
    EXPECT_EQ(graphicsAllocation->memoryType, InternalMemoryType::DEVICE_UNIFIED_MEMORY);
    EXPECT_EQ(graphicsAllocation->gpuAllocation->getGpuAddress(), castToUint64(unfiedMemoryDeviceAllocation));

    retVal = clMemFreeINTEL(&mockContext, unfiedMemoryDeviceAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClSharedMemAllocINTELisCalledWithWrongContextThenInvalidContextErrorIsReturned) {
    cl_int retVal = CL_SUCCESS;
    auto ptr = clSharedMemAllocINTEL(0, 0, nullptr, 0, 0, &retVal);
    EXPECT_EQ(nullptr, ptr);
    EXPECT_EQ(CL_INVALID_CONTEXT, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClSharedMemAllocIntelIsCalledThenItAllocatesSharedUnifiedMemoryAllocation) {
    MockContext mockContext;
    cl_int retVal = CL_SUCCESS;
    auto unfiedMemorySharedAllocation = clSharedMemAllocINTEL(&mockContext, mockContext.getDevice(0u), nullptr, 4, 0, &retVal);
    EXPECT_EQ(CL_SUCCESS, retVal);
    ASSERT_NE(nullptr, unfiedMemorySharedAllocation);

    auto allocationsManager = mockContext.getSVMAllocsManager();
    EXPECT_EQ(1u, allocationsManager->getNumAllocs());
    auto graphicsAllocation = allocationsManager->getSVMAlloc(unfiedMemorySharedAllocation);
    EXPECT_EQ(graphicsAllocation->size, 4u);
    EXPECT_EQ(graphicsAllocation->memoryType, InternalMemoryType::SHARED_UNIFIED_MEMORY);
    EXPECT_EQ(graphicsAllocation->gpuAllocation->getGpuAddress(), castToUint64(unfiedMemorySharedAllocation));

    retVal = clMemFreeINTEL(&mockContext, unfiedMemorySharedAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClMemFreeINTELisCalledWithIncorrectContextThenReturnError) {
    auto retVal = clMemFreeINTEL(0, nullptr);
    EXPECT_EQ(CL_INVALID_CONTEXT, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClMemFreeINTELisCalledWithValidUmPointerThenMemoryIsFreed) {
    MockContext mockContext;
    cl_int retVal = CL_SUCCESS;
    auto unifiedMemoryHostAllocation = clHostMemAllocINTEL(&mockContext, nullptr, 4, 0, &retVal);

    auto allocationsManager = mockContext.getSVMAllocsManager();
    EXPECT_EQ(1u, allocationsManager->getNumAllocs());

    retVal = clMemFreeINTEL(&mockContext, unifiedMemoryHostAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);

    EXPECT_EQ(0u, allocationsManager->getNumAllocs());
}

TEST(clUnifiedSharedMemoryTests, whenClMemFreeINTELisCalledWithInvalidUmPointerThenMemoryIsNotFreed) {
    MockContext mockContext;
    cl_int retVal = CL_SUCCESS;
    auto unifiedMemoryHostAllocation = clHostMemAllocINTEL(&mockContext, nullptr, 4, 0, &retVal);

    auto allocationsManager = mockContext.getSVMAllocsManager();
    EXPECT_EQ(1u, allocationsManager->getNumAllocs());

    retVal = clMemFreeINTEL(&mockContext, ptrOffset(unifiedMemoryHostAllocation, 4));
    EXPECT_EQ(CL_INVALID_VALUE, retVal);

    EXPECT_EQ(1u, allocationsManager->getNumAllocs());
    retVal = clMemFreeINTEL(&mockContext, unifiedMemoryHostAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);

    EXPECT_EQ(0u, allocationsManager->getNumAllocs());
}

TEST(clUnifiedSharedMemoryTests, whenClGetMemAllocInfoINTELisCalledThenOutOfHostMemoryErrorIsReturned) {
    auto retVal = clGetMemAllocInfoINTEL(0, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_EQ(CL_OUT_OF_HOST_MEMORY, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClSetKernelArgMemPointerINTELisCalledWithInvalidKernelThenInvaliKernelErrorIsReturned) {
    auto retVal = clSetKernelArgMemPointerINTEL(0, 0, nullptr);
    EXPECT_EQ(CL_INVALID_KERNEL, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClSetKernelArgMemPointerINTELisCalledWithValidUnifiedMemoryAllocationThenProperFieldsAreSet) {
    auto mockContext = std::make_unique<MockContext>();
    cl_int retVal = CL_SUCCESS;
    auto unfiedMemoryDeviceAllocation = clDeviceMemAllocINTEL(mockContext.get(), mockContext->getDevice(0u), nullptr, 4, 0, &retVal);
    EXPECT_EQ(CL_SUCCESS, retVal);

    MockKernelWithInternals mockKernel(*mockContext->getDevice(0u), mockContext.get(), true);

    retVal = clSetKernelArgMemPointerINTEL(mockKernel.mockKernel, 0, unfiedMemoryDeviceAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);
    auto svmAlloc = mockContext->getSVMAllocsManager()->getSVMAlloc(unfiedMemoryDeviceAllocation);
    EXPECT_EQ(mockKernel.mockKernel->kernelArguments[0].object, svmAlloc->gpuAllocation);

    retVal = clMemFreeINTEL(mockContext.get(), unfiedMemoryDeviceAllocation);
    EXPECT_EQ(CL_SUCCESS, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenclEnqueueMemsetINTELisCalledThenOutOfHostMemoryErrorIsReturned) {
    auto retVal = clEnqueueMemsetINTEL(0, nullptr, 0, 0, 0, nullptr, nullptr);
    EXPECT_EQ(CL_OUT_OF_HOST_MEMORY, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClEnqueueMemcpyINTELisCalledThenOutOfHostMemoryErrorIsReturned) {
    auto retVal = clEnqueueMemcpyINTEL(0, 0, nullptr, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_EQ(CL_OUT_OF_HOST_MEMORY, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClEnqueueMigrateMemINTELisCalledThenOutOfHostMemoryErrorIsReturned) {
    auto retVal = clEnqueueMigrateMemINTEL(0, nullptr, 0, 0, 0, nullptr, nullptr);
    EXPECT_EQ(CL_OUT_OF_HOST_MEMORY, retVal);
}

TEST(clUnifiedSharedMemoryTests, whenClEnqueueMemAdviseINTELisCalledThenOutOfHostMemoryErrorIsReturned) {
    auto retVal = clEnqueueMemAdviseINTEL(0, nullptr, 0, 0, 0, nullptr, nullptr);
    EXPECT_EQ(CL_OUT_OF_HOST_MEMORY, retVal);
}
