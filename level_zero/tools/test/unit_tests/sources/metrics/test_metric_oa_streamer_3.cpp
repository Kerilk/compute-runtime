/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/test/common/test_macros/test.h"
#include "shared/test/common/test_macros/test_base.h"

#include "level_zero/core/source/cmdlist/cmdlist.h"
#include "level_zero/tools/test/unit_tests/sources/metrics/mock_metric_oa.h"

namespace L0 {
namespace ult {

using MetricStreamerMultiDeviceTest = Test<MetricStreamerMultiDeviceFixture>;

TEST_F(MetricStreamerMultiDeviceTest, givenEnableWalkerPartitionIsOnWhenZetCommandListAppendMetricStreamerMarkerIsCalledForSubDeviceThenReturnsSuccess) {

    DebugManagerStateRestore restorer;
    DebugManager.flags.EnableWalkerPartition.set(1);

    auto &deviceImp = *static_cast<DeviceImp *>(devices[0]);
    zet_device_handle_t metricDeviceHandle = deviceImp.subDevices[0]->toHandle();

    ze_event_handle_t eventHandle = {};

    ze_result_t returnValue;
    std::unique_ptr<L0::CommandList> commandList(CommandList::create(productFamily, deviceImp.subDevices[0], NEO::EngineGroupType::RenderCompute, 0u, returnValue));

    zet_metric_streamer_handle_t streamerHandle = {};
    zet_metric_streamer_desc_t streamerDesc = {};

    streamerDesc.stype = ZET_STRUCTURE_TYPE_METRIC_STREAMER_DESC;
    streamerDesc.notifyEveryNReports = 32768;
    streamerDesc.samplingPeriod = 1000;

    Mock<MetricGroup> metricGroup;
    zet_metric_group_handle_t metricGroupHandle = metricGroup.toHandle();
    zet_metric_group_properties_t metricGroupProperties = {ZET_STRUCTURE_TYPE_METRIC_GROUP_PROPERTIES, nullptr};

    metricsDeviceParams.ConcurrentGroupsCount = 1;

    Mock<IConcurrentGroup_1_5> metricsConcurrentGroup;
    TConcurrentGroupParams_1_0 metricsConcurrentGroupParams = {};
    metricsConcurrentGroupParams.MetricSetsCount = 1;
    metricsConcurrentGroupParams.SymbolName = "OA";
    metricsConcurrentGroupParams.Description = "OA description";

    Mock<MetricsDiscovery::IMetricSet_1_5> metricsSet;
    MetricsDiscovery::TMetricSetParams_1_4 metricsSetParams = {};
    metricsSetParams.ApiMask = MetricsDiscovery::API_TYPE_IOSTREAM;
    metricsSetParams.MetricsCount = 0;
    metricsSetParams.SymbolName = "Metric set name";
    metricsSetParams.ShortName = "Metric set description";
    metricsSetParams.RawReportSize = 256;

    TypedValue_1_0 value = {};
    value.Type = ValueType::Uint32;
    value.ValueUInt32 = 64;

    ContextHandle_1_0 contextHandle = {&value};

    CommandBufferSize_1_0 commandBufferSize = {};
    commandBufferSize.GpuMemorySize = 100;

    openMetricsAdapterSubDevice(0);

    mockMetricsLibrarySubDevices[0]->g_mockApi->contextCreateOutHandle = contextHandle;
    mockMetricsLibrarySubDevices[0]->g_mockApi->commandBufferGetSizeOutSize = commandBufferSize;

    setupDefaultMocksForMetricDevice(metricsDevice);

    metricsDevice.getConcurrentGroupResults.push_back(&metricsConcurrentGroup);

    metricsConcurrentGroup.GetParamsResult = &metricsConcurrentGroupParams;
    metricsConcurrentGroup.getMetricSetResult = &metricsSet;

    metricsSet.GetParamsResult = &metricsSetParams;

    uint32_t metricGroupCount = 0;
    EXPECT_EQ(zetMetricGroupGet(metricDeviceHandle, &metricGroupCount, nullptr), ZE_RESULT_SUCCESS);
    EXPECT_EQ(metricGroupCount, 1u);

    EXPECT_EQ(zetMetricGroupGet(metricDeviceHandle, &metricGroupCount, &metricGroupHandle), ZE_RESULT_SUCCESS);
    EXPECT_EQ(metricGroupCount, 1u);
    EXPECT_NE(metricGroupHandle, nullptr);

    EXPECT_EQ(zetMetricGroupGetProperties(metricGroupHandle, &metricGroupProperties), ZE_RESULT_SUCCESS);
    EXPECT_EQ(zetContextActivateMetricGroups(context->toHandle(), metricDeviceHandle, 1, &metricGroupHandle), ZE_RESULT_SUCCESS);
    EXPECT_EQ(zetMetricStreamerOpen(context->toHandle(), metricDeviceHandle, metricGroupHandle, &streamerDesc, eventHandle, &streamerHandle), ZE_RESULT_SUCCESS);
    EXPECT_NE(streamerHandle, nullptr);

    std::array<uint32_t, 10> markerValues = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    for (auto &markerValue : markerValues) {
        EXPECT_EQ(zetCommandListAppendMetricStreamerMarker(commandList->toHandle(), streamerHandle, markerValue), ZE_RESULT_SUCCESS);
    }

    EXPECT_EQ(zetMetricStreamerClose(streamerHandle), ZE_RESULT_SUCCESS);
}

TEST_F(MetricStreamerMultiDeviceTest, givenValidArgumentsWhenZetMetricGroupCalculateMetricValuesExpThenReturnsSuccess) {

    zet_device_handle_t metricDeviceHandle = devices[0]->toHandle();
    auto &deviceImp = *static_cast<DeviceImp *>(devices[0]);
    const uint32_t subDeviceCount = static_cast<uint32_t>(deviceImp.subDevices.size());

    ze_event_handle_t eventHandle = {};

    zet_metric_streamer_handle_t streamerHandle = {};
    zet_metric_streamer_desc_t streamerDesc = {};

    streamerDesc.stype = ZET_STRUCTURE_TYPE_METRIC_STREAMER_DESC;
    streamerDesc.notifyEveryNReports = 32768;
    streamerDesc.samplingPeriod = 1000;

    Mock<MetricGroup> metricGroup;
    zet_metric_group_handle_t metricGroupHandle = metricGroup.toHandle();

    metricsDeviceParams.ConcurrentGroupsCount = 1;

    Mock<IConcurrentGroup_1_5> metricsConcurrentGroup;
    TConcurrentGroupParams_1_0 metricsConcurrentGroupParams = {};
    metricsConcurrentGroupParams.MetricSetsCount = 1;
    metricsConcurrentGroupParams.SymbolName = "OA";
    metricsConcurrentGroupParams.Description = "OA description";

    Mock<MetricsDiscovery::IMetricSet_1_5> metricsSet;
    MetricsDiscovery::TMetricSetParams_1_4 metricsSetParams = {};
    metricsSetParams.ApiMask = MetricsDiscovery::API_TYPE_IOSTREAM;
    metricsSetParams.SymbolName = "Metric set name";
    metricsSetParams.ShortName = "Metric set description";
    metricsSetParams.RawReportSize = 256;
    metricsSetParams.MetricsCount = 11;

    Mock<IMetric_1_0> metric;
    MetricsDiscovery::TMetricParams_1_0 metricParams = {};

    uint32_t returnedMetricCount = 1;

    openMetricsAdapter();

    setupDefaultMocksForMetricDevice(metricsDevice);

    metricsDevice.getConcurrentGroupResults.push_back(&metricsConcurrentGroup);

    metricsConcurrentGroup.GetParamsResult = &metricsConcurrentGroupParams;
    metricsConcurrentGroup.getMetricSetResult = &metricsSet;

    metricsSet.GetParamsResult = &metricsSetParams;
    metricsSet.GetMetricResult = &metric;
    metricsSet.calculateMetricsOutReportCount = &returnedMetricCount;

    metric.GetParamsResult = &metricParams;

    uint32_t metricGroupCount = 0;
    EXPECT_EQ(zetMetricGroupGet(metricDeviceHandle, &metricGroupCount, nullptr), ZE_RESULT_SUCCESS);
    EXPECT_EQ(metricGroupCount, 1u);

    EXPECT_EQ(zetMetricGroupGet(metricDeviceHandle, &metricGroupCount, &metricGroupHandle), ZE_RESULT_SUCCESS);
    EXPECT_EQ(metricGroupCount, 1u);
    EXPECT_NE(metricGroupHandle, nullptr);

    EXPECT_EQ(zetContextActivateMetricGroups(context->toHandle(), metricDeviceHandle, 1, &metricGroupHandle), ZE_RESULT_SUCCESS);

    EXPECT_EQ(zetMetricStreamerOpen(context->toHandle(), metricDeviceHandle, metricGroupHandle, &streamerDesc, eventHandle, &streamerHandle), ZE_RESULT_SUCCESS);
    EXPECT_NE(streamerHandle, nullptr);

    size_t rawSize = 0;
    uint32_t reportCount = 256;
    EXPECT_EQ(zetMetricStreamerReadData(streamerHandle, reportCount, &rawSize, nullptr), ZE_RESULT_SUCCESS);

    std::vector<uint8_t> rawData;
    rawData.resize(rawSize);
    size_t rawRequestSize = rawSize;
    EXPECT_EQ(zetMetricStreamerReadData(streamerHandle, reportCount, &rawSize, rawData.data()), ZE_RESULT_SUCCESS);
    EXPECT_EQ(rawSize, rawRequestSize);

    uint32_t dataCount = 0;
    uint32_t totalMetricCount = 0;
    EXPECT_EQ(L0::zetMetricGroupCalculateMultipleMetricValuesExp(metricGroupHandle, ZET_METRIC_GROUP_CALCULATION_TYPE_METRIC_VALUES, rawSize, rawData.data(), &dataCount, &totalMetricCount, nullptr, nullptr), ZE_RESULT_SUCCESS);
    EXPECT_EQ(totalMetricCount, subDeviceCount * metricsSetParams.MetricsCount * reportCount);
    ASSERT_GE(dataCount, 2u);

    std::vector<uint32_t> metricCounts(dataCount);
    std::vector<zet_typed_value_t> caculatedRawResults(totalMetricCount);
    EXPECT_EQ(L0::zetMetricGroupCalculateMultipleMetricValuesExp(metricGroupHandle, ZET_METRIC_GROUP_CALCULATION_TYPE_METRIC_VALUES, rawSize, rawData.data(), &dataCount, &totalMetricCount, metricCounts.data(), caculatedRawResults.data()), ZE_RESULT_SUCCESS);
    EXPECT_EQ(metricCounts[0], metricsSetParams.MetricsCount);
    EXPECT_EQ(metricCounts[1], metricsSetParams.MetricsCount);

    EXPECT_EQ(zetMetricStreamerClose(streamerHandle), ZE_RESULT_SUCCESS);
}

using MetricStreamerTest = Test<MetricContextFixture>;
TEST_F(MetricStreamerTest, givenRawReportSizeIsNotAlignedToOaBufferSizeWhenZetMetricStreamerReadDataIsCalledThenReadSizeIsAlignedToRawReportSize) {

    zet_device_handle_t metricDeviceHandle = device->toHandle();
    ze_event_handle_t eventHandle = {};
    zet_metric_streamer_handle_t streamerHandle = {};
    zet_metric_streamer_desc_t streamerDesc = {};
    streamerDesc.stype = ZET_STRUCTURE_TYPE_METRIC_STREAMER_DESC;
    streamerDesc.notifyEveryNReports = 32768;
    streamerDesc.samplingPeriod = 1000;
    Mock<MetricGroup> metricGroup;
    zet_metric_group_handle_t metricGroupHandle = metricGroup.toHandle();
    metricsDeviceParams.ConcurrentGroupsCount = 1;

    Mock<IConcurrentGroup_1_5> metricsConcurrentGroup;
    TConcurrentGroupParams_1_0 metricsConcurrentGroupParams = {};
    metricsConcurrentGroupParams.MetricSetsCount = 1;
    metricsConcurrentGroupParams.SymbolName = "OA";
    metricsConcurrentGroupParams.Description = "OA description";

    Mock<MetricsDiscovery::IMetricSet_1_5> metricsSet;
    MetricsDiscovery::TMetricSetParams_1_4 metricsSetParams = {};
    metricsSetParams.ApiMask = MetricsDiscovery::API_TYPE_IOSTREAM;
    metricsSetParams.MetricsCount = 0;
    metricsSetParams.SymbolName = "Metric set name";
    metricsSetParams.ShortName = "Metric set description";
    metricsSetParams.RawReportSize = 576;

    uint32_t testOaBufferSize = 128 * MB;

    openMetricsAdapter();

    setupDefaultMocksForMetricDevice(metricsDevice);

    metricsDevice.getConcurrentGroupResults.push_back(&metricsConcurrentGroup);

    metricsConcurrentGroup.GetParamsResult = &metricsConcurrentGroupParams;
    metricsConcurrentGroup.getMetricSetResult = &metricsSet;

    metricsSet.GetParamsResult = &metricsSetParams;

    metricsConcurrentGroup.openIoStreamOutOaBufferSize = &testOaBufferSize;

    uint32_t metricGroupCount = 0;
    EXPECT_EQ(zetMetricGroupGet(metricDeviceHandle, &metricGroupCount, nullptr), ZE_RESULT_SUCCESS);
    EXPECT_EQ(zetMetricGroupGet(metricDeviceHandle, &metricGroupCount, &metricGroupHandle), ZE_RESULT_SUCCESS);
    EXPECT_NE(metricGroupHandle, nullptr);
    EXPECT_EQ(zetContextActivateMetricGroups(context->toHandle(), metricDeviceHandle, 1, &metricGroupHandle), ZE_RESULT_SUCCESS);
    EXPECT_EQ(zetMetricStreamerOpen(context->toHandle(), metricDeviceHandle, metricGroupHandle, &streamerDesc, eventHandle, &streamerHandle), ZE_RESULT_SUCCESS);

    size_t rawSize = 0;
    uint32_t reportCount = std::numeric_limits<uint32_t>::max();
    EXPECT_EQ(zetMetricStreamerReadData(streamerHandle, reportCount, &rawSize, nullptr), ZE_RESULT_SUCCESS);
    EXPECT_LE(rawSize, testOaBufferSize);
    EXPECT_EQ(0u, rawSize % metricsSetParams.RawReportSize);
    EXPECT_EQ(zetMetricStreamerClose(streamerHandle), ZE_RESULT_SUCCESS);
}

} // namespace ult
} // namespace L0
