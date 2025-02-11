/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "level_zero/sysman/source/driver/sysman_driver_handle.h"

#include <string>
#include <unordered_map>

namespace L0 {
namespace Sysman {
struct SysmanDevice;
struct SysmanDriverHandleImp : SysmanDriverHandle {
    ~SysmanDriverHandleImp() override;
    SysmanDriverHandleImp();
    ze_result_t initialize(NEO::ExecutionEnvironment &executionEnvironment);
    ze_result_t getDevice(uint32_t *pCount, zes_device_handle_t *phDevices) override;
    ze_result_t getExtensionProperties(uint32_t *pCount, zes_driver_extension_properties_t *pExtensionProperties) override;
    ze_result_t sysmanEventsListen(uint32_t timeout, uint32_t count, zes_device_handle_t *phDevices,
                                   uint32_t *pNumDeviceEvents, zes_event_type_flags_t *pEvents) override;
    ze_result_t sysmanEventsListenEx(uint64_t timeout, uint32_t count, zes_device_handle_t *phDevices,
                                     uint32_t *pNumDeviceEvents, zes_event_type_flags_t *pEvents) override;
    std::vector<SysmanDevice *> sysmanDevices;
    uint32_t numDevices = 0;
    ze_result_t getExtensionFunctionAddress(const char *pFuncName, void **pfunc) override;
    std::unordered_map<std::string, void *> extensionFunctionsLookupMap;
    struct OsSysmanDriver *pOsSysmanDriver = nullptr;
};

extern struct SysmanDriverHandleImp *GlobalSysmanDriver;

} // namespace Sysman
} // namespace L0
