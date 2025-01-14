/*
 * Copyright (C) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "debug_settings_manager.h"

#include "shared/source/debug_settings/debug_variables_helper.h"
#include "shared/source/debug_settings/definitions/translate_debug_settings.h"
#include "shared/source/helpers/api_specific_config.h"
#include "shared/source/helpers/debug_helpers.h"
#include "shared/source/helpers/string.h"
#include "shared/source/utilities/debug_settings_reader_creator.h"
#include "shared/source/utilities/io_functions.h"
#include "shared/source/utilities/logger.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace NEO {

template <typename T>
static std::string toString(const T &arg) {
    if constexpr (std::is_convertible_v<std::string, T>) {
        return static_cast<std::string>(arg);
    } else {
        return std::to_string(arg);
    }
}

template <DebugFunctionalityLevel DebugLevel>
DebugSettingsManager<DebugLevel>::DebugSettingsManager(const char *registryPath) {
    readerImpl = SettingsReaderCreator::create(std::string(registryPath));
    ApiSpecificConfig::initPrefixes();
    injectSettingsFromReader();
    dumpFlags();
    translateDebugSettings(flags);

    while (isLoopAtDriverInitEnabled())
        ;
}

template <DebugFunctionalityLevel DebugLevel>
DebugSettingsManager<DebugLevel>::~DebugSettingsManager() {
    readerImpl.reset();
};

template <DebugFunctionalityLevel DebugLevel>
void DebugSettingsManager<DebugLevel>::getHardwareInfoOverride(std::string &hwInfoConfig) {
    std::string str = flags.HardwareInfoOverride.get();
    if (str[0] == '\"') {
        str.pop_back();
        hwInfoConfig = str.substr(1, std::string::npos);
    } else {
        hwInfoConfig = str;
    }
}

static const char *convPrefixToString(DebugVarPrefix prefix) {
    if (prefix == DebugVarPrefix::Neo) {
        return "NEO_";
    } else if (prefix == DebugVarPrefix::Neo_L0) {
        return "NEO_L0_";
    } else if (prefix == DebugVarPrefix::Neo_Ocl) {
        return "NEO_OCL_";
    } else {
        return "";
    }
}

template <DebugFunctionalityLevel DebugLevel>
template <typename DataType>
void DebugSettingsManager<DebugLevel>::dumpNonDefaultFlag(const char *variableName, const DataType &variableValue, const DataType &defaultValue, std::ostringstream &ostring) {
    if (variableValue != defaultValue) {
        const auto variableStringValue = toString(variableValue);
        ostring << "Non-default value of debug variable: " << variableName << " = " << variableStringValue.c_str() << '\n';
    }
}

template <DebugFunctionalityLevel DebugLevel>
void DebugSettingsManager<DebugLevel>::getStringWithFlags(std::string &allFlags, std::string &changedFlags) const {
    std::ostringstream allFlagsStream;
    allFlagsStream.str("");

    std::ostringstream changedFlagsStream;
    changedFlagsStream.str("");

#define DECLARE_DEBUG_VARIABLE(dataType, variableName, defaultValue, description)                                 \
    {                                                                                                             \
        std::string neoKey = convPrefixToString(flags.variableName.getPrefixType());                              \
        neoKey += getNonReleaseKeyName(#variableName);                                                            \
        allFlagsStream << neoKey.c_str() << " = " << flags.variableName.get() << '\n';                            \
        dumpNonDefaultFlag<dataType>(neoKey.c_str(), flags.variableName.get(), defaultValue, changedFlagsStream); \
    }
    if (registryReadAvailable() || isDebugKeysReadEnabled()) {
#include "debug_variables.inl"
    }
#undef DECLARE_DEBUG_VARIABLE

#define DECLARE_DEBUG_VARIABLE(dataType, variableName, defaultValue, description)                                 \
    {                                                                                                             \
        std::string neoKey = convPrefixToString(flags.variableName.getPrefixType());                              \
        neoKey += #variableName;                                                                                  \
        allFlagsStream << neoKey.c_str() << " = " << flags.variableName.get() << '\n';                            \
        dumpNonDefaultFlag<dataType>(neoKey.c_str(), flags.variableName.get(), defaultValue, changedFlagsStream); \
    }
#include "release_variables.inl"
#undef DECLARE_DEBUG_VARIABLE

    allFlags = allFlagsStream.str();
    changedFlags = changedFlagsStream.str();
}

template <DebugFunctionalityLevel DebugLevel>
void DebugSettingsManager<DebugLevel>::dumpFlags() const {
    if (flags.PrintDebugSettings.get() == false) {
        return;
    }

    std::ofstream settingsDumpFile{settingsDumpFileName, std::ios::out};
    DEBUG_BREAK_IF(!settingsDumpFile.good());

    std::string allFlags;
    std::string changedFlags;

    getStringWithFlags(allFlags, changedFlags);
    PRINT_DEBUG_STRING(true, stdout, "%s", changedFlags.c_str());

    settingsDumpFile << allFlags;
}

template <DebugFunctionalityLevel DebugLevel>
void DebugSettingsManager<DebugLevel>::injectSettingsFromReader() {
#undef DECLARE_DEBUG_VARIABLE
#define DECLARE_DEBUG_VARIABLE(dataType, variableName, defaultValue, description)                                        \
    {                                                                                                                    \
        DebugVarPrefix type;                                                                                             \
        dataType tempData = readerImpl->getSetting(getNonReleaseKeyName(#variableName), flags.variableName.get(), type); \
        flags.variableName.setPrefixType(type);                                                                          \
        flags.variableName.set(tempData);                                                                                \
    }

    if (registryReadAvailable() || isDebugKeysReadEnabled()) {
#include "debug_variables.inl"
    }

#undef DECLARE_DEBUG_VARIABLE
#define DECLARE_DEBUG_VARIABLE(dataType, variableName, defaultValue, description)                  \
    {                                                                                              \
        DebugVarPrefix type;                                                                       \
        dataType tempData = readerImpl->getSetting(#variableName, flags.variableName.get(), type); \
        flags.variableName.setPrefixType(type);                                                    \
        flags.variableName.set(tempData);                                                          \
    }
#include "release_variables.inl"
#undef DECLARE_DEBUG_VARIABLE
}

void logDebugString(std::string_view debugString) {
    NEO::fileLoggerInstance().logDebugString(true, debugString);
}

template class DebugSettingsManager<DebugFunctionalityLevel::None>;
template class DebugSettingsManager<DebugFunctionalityLevel::Full>;
template class DebugSettingsManager<DebugFunctionalityLevel::RegKeys>;
}; // namespace NEO
