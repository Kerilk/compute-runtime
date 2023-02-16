/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::vector<uint8_t> compileToSpirV(const std::string &src, const std::string &options, std::string &outCompilerLog);

extern const char *memcpyBytesTestKernelSrc;

extern const char *memcpyBytesWithPrintfTestKernelSrc;
