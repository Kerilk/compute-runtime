/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "shared/source/helpers/hw_info.h"
#include "shared/source/xe_hpc_core/hw_cmds_xe_hpc_core_base.h"

namespace NEO {

struct PVC : public XeHpcCoreFamily {
    static const PLATFORM platform;
    static const HardwareInfo hwInfo;
    static FeatureTable featureTable;
    static WorkaroundTable workaroundTable;
    // Initial non-zero values for unit tests
    static const uint32_t maxEuPerSubslice = 8;
    static const uint32_t maxSlicesSupported = 8;
    static const uint32_t maxSubslicesSupported = 64;
    static const uint32_t maxDualSubslicesSupported = 64;
    static const RuntimeCapabilityTable capabilityTable;
    static constexpr uint32_t numberOfpartsInTileForConcurrentKernels = 8u;

    struct FrontEndStateSupport {
        static constexpr bool scratchSize = true;
        static constexpr bool privateScratchSize = true;
        static constexpr bool computeDispatchAllWalker = true;
        static constexpr bool disableEuFusion = false;
        static constexpr bool disableOverdispatch = true;
        static constexpr bool singleSliceDispatchCcsMode = true;
    };

    struct StateComputeModeStateSupport {
        static constexpr bool threadArbitrationPolicy = true;
        static constexpr bool coherencyRequired = true;
        static constexpr bool largeGrfMode = true;
        static constexpr bool zPassAsyncComputeThreadLimit = false;
        static constexpr bool pixelAsyncComputeThreadLimit = false;
        static constexpr bool devicePreemptionMode = false;
    };

    static void (*setupHardwareInfo)(HardwareInfo *hwInfo, bool setupFeatureTableAndWorkaroundTable, uint64_t hwInfoConfig, const CompilerProductHelper &compilerProductHelper);
    static void setupFeatureAndWorkaroundTable(HardwareInfo *hwInfo);
    static void setupHardwareInfoBase(HardwareInfo *hwInfo, bool setupFeatureTableAndWorkaroundTable, const CompilerProductHelper &compilerProductHelper);
    static void setupHardwareInfoMultiTileBase(HardwareInfo *hwInfo, bool setupMultiTile);
    static void adjustHardwareInfo(HardwareInfo *hwInfo);

    static constexpr uint8_t pvcBaseDieRevMask = 0b111000; // [3:5]
    static constexpr uint8_t pvcBaseDieA0Masked = 0;       // [3:5] == 0

    static bool isXl(const HardwareInfo &hwInfo);

    static bool isXt(const HardwareInfo &hwInfo);

    static bool isXlA0(const HardwareInfo &hwInfo) {
        auto revId = hwInfo.platform.usRevId & pvcSteppingBits;
        return (revId < 0x3);
    }

    static bool isAtMostXtA0(const HardwareInfo &hwInfo) {
        auto revId = hwInfo.platform.usRevId & pvcSteppingBits;
        return (revId <= 0x3);
    }
    static constexpr uint32_t pvcSteppingBits = 0b111;
};

class PvcHwConfig : public PVC {
  public:
    static void setupHardwareInfo(HardwareInfo *hwInfo, bool setupFeatureTableAndWorkaroundTable, const CompilerProductHelper &compilerProductHelper);
    static const HardwareInfo hwInfo;

  private:
    static GT_SYSTEM_INFO gtSystemInfo;
};

} // namespace NEO
