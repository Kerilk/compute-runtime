/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "shared/source/os_interface/os_library.h"

class MockOsLibrary : public NEO::OsLibrary {
  public:
    MockOsLibrary(void *procAddress, bool isLoaded) : getProcAddressReturn{procAddress}, isLoadedReturn{isLoaded} {}
    MockOsLibrary() {}

    std::string lastRequestedProcName;
    void *getProcAddressReturn = nullptr;

    void *getProcAddress(const std::string &procName) override {
        lastRequestedProcName = procName;
        return getProcAddressReturn;
    }

    bool isLoadedReturn = false;

    bool isLoaded() override {
        return isLoadedReturn;
    }

    std::string getFullPath() override {
        return std::string();
    }

    static OsLibrary *loadLibraryNewObject;

    static OsLibrary *load(const std::string &name) {
        OsLibrary *ptr = loadLibraryNewObject;
        loadLibraryNewObject = nullptr;
        return ptr;
    }
};
