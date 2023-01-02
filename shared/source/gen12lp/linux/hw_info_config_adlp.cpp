/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/gen12lp/hw_cmds_adlp.h"
#include "shared/source/helpers/hw_info.h"
#include "shared/source/helpers/pipeline_select_args.h"
#include "shared/source/helpers/pipeline_select_helper.h"
#include "shared/source/os_interface/hw_info_config.h"
#include "shared/source/os_interface/hw_info_config.inl"
#include "shared/source/os_interface/hw_info_config_bdw_and_later.inl"

#include "platforms.h"

constexpr static auto gfxProduct = IGFX_ALDERLAKE_P;

#include "shared/source/gen12lp/adlp/os_agnostic_hw_info_config_adlp.inl"
#include "shared/source/gen12lp/os_agnostic_hw_info_config_gen12lp.inl"
namespace NEO {

template <>
int ProductHelperHw<gfxProduct>::configureHardwareCustom(HardwareInfo *hwInfo, OSInterface *osIface) const {
    GT_SYSTEM_INFO *gtSystemInfo = &hwInfo->gtSystemInfo;
    gtSystemInfo->SliceCount = 1;

    hwInfo->featureTable.flags.ftrGpGpuMidThreadLevelPreempt = (hwInfo->platform.usRevId >= this->getHwRevIdFromStepping(REVISION_B, *hwInfo));

    enableBlitterOperationsSupport(hwInfo);

    return 0;
}

template class ProductHelperHw<gfxProduct>;
} // namespace NEO
