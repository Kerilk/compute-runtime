/*
 * Copyright (C) 2021-2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "shared/source/utilities/const_stringref.h"

namespace NEO {
namespace Queries {
constexpr ConstStringRef queryNeoRevision = "NEO_REVISION";
constexpr ConstStringRef queryOCLDriverVersion = "OCL_DRIVER_VERSION";
}; // namespace Queries
} // namespace NEO
