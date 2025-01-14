/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/aub_mem_dump/definitions/aub_services.h"
#include "shared/source/command_stream/preemption_mode.h"
#include "shared/source/helpers/compiler_product_helper.h"
#include "shared/source/helpers/constants.h"
#include "shared/source/xe_hpg_core/hw_cmds_arl.h"

#include "aubstream/engine_node.h"

namespace NEO {

const char *HwMapper<IGFX_ARROWLAKE>::abbreviation = "arl";

const PLATFORM ARL::platform = {
    IGFX_ARROWLAKE,
    PCH_UNKNOWN,
    IGFX_XE_HPG_CORE,
    IGFX_XE_HPG_CORE,
    PLATFORM_NONE, // default init
    0,             // usDeviceID
    0,             // usRevId. 0 sets the stepping to A0
    0,             // usDeviceID_PCH
    0,             // usRevId_PCH
    GTTYPE_UNDEFINED};

const RuntimeCapabilityTable ARL::capabilityTable{
    EngineDirectSubmissionInitVec{
        {aub_stream::ENGINE_CCS, {true, false, false, true}}}, // directSubmissionEngines
    {0, 0, 0, 0, false, false, false, false},                  // kmdNotifyProperties
    MemoryConstants::max48BitAddress,                          // gpuAddressSpace
    0,                                                         // sharedSystemMemCapabilities
    83.333,                                                    // defaultProfilingTimerResolution
    MemoryConstants::pageSize,                                 // requiredPreemptionSurfaceSize
    "",                                                        // deviceName
    nullptr,                                                   // preferredPlatformName
    PreemptionMode::ThreadGroup,                               // defaultPreemptionMode
    aub_stream::ENGINE_CCS,                                    // defaultEngineType
    0,                                                         // maxRenderFrequency
    30,                                                        // clVersionSupport
    CmdServicesMemTraceVersion::DeviceValues::Arl,             // aubDeviceId
    0,                                                         // extraQuantityThreadsPerEU
    64,                                                        // slmSize
    sizeof(ARL::GRF),                                          // grfSize
    36u,                                                       // timestampValidBits
    32u,                                                       // kernelTimestampValidBits
    false,                                                     // blitterOperationsSupported
    true,                                                      // ftrSupportsInteger64BitAtomics
    true,                                                      // ftrSupportsFP64
    false,                                                     // ftrSupportsFP64Emulation
    true,                                                      // ftrSupports64BitMath
    true,                                                      // ftrSvm
    false,                                                     // ftrSupportsCoherency
    false,                                                     // ftrSupportsVmeAvcTextureSampler
    false,                                                     // ftrSupportsVmeAvcPreemption
    false,                                                     // ftrRenderCompressedBuffers
    false,                                                     // ftrRenderCompressedImages
    true,                                                      // ftr64KBpages
    true,                                                      // instrumentationEnabled
    false,                                                     // supportsVme
    true,                                                      // supportCacheFlushAfterWalker
    true,                                                      // supportsImages
    false,                                                     // supportsDeviceEnqueue
    false,                                                     // supportsPipes
    true,                                                      // supportsOcl21Features
    false,                                                     // supportsOnDemandPageFaults
    false,                                                     // supportsIndependentForwardProgress
    false,                                                     // hostPtrTrackingEnabled
    true,                                                      // levelZeroSupported
    true,                                                      // isIntegratedDevice
    true,                                                      // supportsMediaBlock
    false,                                                     // p2pAccessSupported
    false,                                                     // p2pAtomicAccessSupported
    true,                                                      // fusedEuEnabled
    false,                                                     // l0DebuggerSupported
    true                                                       // supportsFloatAtomics
};

WorkaroundTable ARL::workaroundTable = {};
FeatureTable ARL::featureTable = {};

void ARL::setupFeatureAndWorkaroundTable(HardwareInfo *hwInfo) {
    FeatureTable *featureTable = &hwInfo->featureTable;
    WorkaroundTable *workaroundTable = &hwInfo->workaroundTable;

    featureTable->flags.ftrL3IACoherency = true;
    featureTable->flags.ftrPPGTT = true;
    featureTable->flags.ftrSVM = true;
    featureTable->flags.ftrIA32eGfxPTEs = true;
    featureTable->flags.ftrStandardMipTailFormat = true;
    featureTable->flags.ftrTranslationTable = true;
    featureTable->flags.ftrUserModeTranslationTable = true;
    featureTable->flags.ftrTileMappedResource = true;
    featureTable->flags.ftrFbc = true;
    featureTable->flags.ftrAstcHdr2D = true;
    featureTable->flags.ftrAstcLdr2D = true;

    featureTable->flags.ftrGpGpuMidBatchPreempt = true;
    featureTable->flags.ftrGpGpuThreadGroupLevelPreempt = true;

    featureTable->flags.ftrTileY = false;
    featureTable->flags.ftrLinearCCS = true;
    featureTable->flags.ftrE2ECompression = false;
    featureTable->flags.ftrCCSNode = true;
    featureTable->flags.ftrCCSRing = true;
    featureTable->flags.ftrTile64Optimization = true;

    workaroundTable->flags.wa4kAlignUVOffsetNV12LinearSurface = true;
    workaroundTable->flags.waUntypedBufferCompression = true;
};

void ARL::setupHardwareInfoBase(HardwareInfo *hwInfo, bool setupFeatureTableAndWorkaroundTable, const CompilerProductHelper &compilerProductHelper) {
    GT_SYSTEM_INFO *gtSysInfo = &hwInfo->gtSystemInfo;
    gtSysInfo->ThreadCount = gtSysInfo->EUCount * compilerProductHelper.getNumThreadsPerEu();
    gtSysInfo->TotalPsThreadsWindowerRange = 64;
    gtSysInfo->CsrSizeInMb = 8;
    gtSysInfo->IsL3HashModeEnabled = false;
    gtSysInfo->IsDynamicallyPopulated = false;
    gtSysInfo->TotalVsThreads = 336;
    gtSysInfo->TotalHsThreads = 336;
    gtSysInfo->TotalDsThreads = 336;
    gtSysInfo->TotalGsThreads = 336;

    gtSysInfo->CCSInfo.IsValid = true;
    gtSysInfo->CCSInfo.NumberOfCCSEnabled = 1;
    gtSysInfo->CCSInfo.Instances.CCSEnableMask = 0b1;

    if (setupFeatureTableAndWorkaroundTable) {
        setupFeatureAndWorkaroundTable(hwInfo);
    }
}

const HardwareInfo ArlHwConfig::hwInfo = {
    &ARL::platform,
    &ARL::featureTable,
    &ARL::workaroundTable,
    &ArlHwConfig::gtSystemInfo,
    ARL::capabilityTable};

GT_SYSTEM_INFO ArlHwConfig::gtSystemInfo = {0};
void ArlHwConfig::setupHardwareInfo(HardwareInfo *hwInfo, bool setupFeatureTableAndWorkaroundTable, const CompilerProductHelper &compilerProductHelper) {
    ARL::setupHardwareInfoBase(hwInfo, setupFeatureTableAndWorkaroundTable, compilerProductHelper);
    GT_SYSTEM_INFO *gtSysInfo = &hwInfo->gtSystemInfo;
    gtSysInfo->CsrSizeInMb = 8;
    gtSysInfo->IsL3HashModeEnabled = false;
    gtSysInfo->IsDynamicallyPopulated = false;

    // non-zero values for unit tests
    if (gtSysInfo->SliceCount == 0) {
        gtSysInfo->SliceCount = 1;
        gtSysInfo->SubSliceCount = 2;
        gtSysInfo->DualSubSliceCount = gtSysInfo->SubSliceCount;
        gtSysInfo->EUCount = 4;
        gtSysInfo->MaxEuPerSubSlice = gtSysInfo->EUCount / gtSysInfo->SubSliceCount;
        gtSysInfo->MaxSlicesSupported = gtSysInfo->SliceCount;
        gtSysInfo->MaxSubSlicesSupported = gtSysInfo->SubSliceCount;

        gtSysInfo->L3BankCount = 1;

        hwInfo->featureTable.ftrBcsInfo = 1;
        gtSysInfo->IsDynamicallyPopulated = true;
        for (uint32_t slice = 0; slice < gtSysInfo->SliceCount; slice++) {
            gtSysInfo->SliceInfo[slice].Enabled = true;
        }
    }
    gtSysInfo->L3CacheSizeInKb = 1;

    if (setupFeatureTableAndWorkaroundTable) {
        ARL::setupFeatureAndWorkaroundTable(hwInfo);
    }
};

const HardwareInfo ARL::hwInfo = ArlHwConfig::hwInfo;

void setupARLHardwareInfoImpl(HardwareInfo *hwInfo, bool setupFeatureTableAndWorkaroundTable, uint64_t hwInfoConfig, const CompilerProductHelper &compilerProductHelper) {
    ArlHwConfig::setupHardwareInfo(hwInfo, setupFeatureTableAndWorkaroundTable, compilerProductHelper);
}

void (*ARL::setupHardwareInfo)(HardwareInfo *, bool, uint64_t, const CompilerProductHelper &) = setupARLHardwareInfoImpl;
} // namespace NEO
