/*
 * Copyright (C) 2022-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/ail/ail_configuration_base.inl"
#include "shared/source/helpers/hw_info.h"

#include "aubstream/engine_node.h"

#include <map>
#include <vector>

namespace NEO {

extern std::map<std::string_view, std::vector<AILEnumeration>> applicationMapMTL;

static EnableAIL<IGFX_METEORLAKE> enableAILMTL;

template <>
void AILConfigurationHw<IGFX_METEORLAKE>::applyExt(RuntimeCapabilityTable &runtimeCapabilityTable) {
    auto search = applicationMapMTL.find(processName);
    if (search != applicationMapMTL.end()) {
        for (size_t i = 0; i < search->second.size(); ++i) {
            switch (search->second[i]) {
            case AILEnumeration::DISABLE_DIRECT_SUBMISSION:
                runtimeCapabilityTable.directSubmissionEngines.data[aub_stream::ENGINE_CCS].engineSupported = false;
            default:
                break;
            }
        }
    }
}

template <>
bool AILConfigurationHw<IGFX_METEORLAKE>::isContextSyncFlagRequired() {
    auto iterator = applicationsContextSyncFlag.find(processName);
    return iterator != applicationsContextSyncFlag.end();
}

template class AILConfigurationHw<IGFX_METEORLAKE>;

} // namespace NEO
