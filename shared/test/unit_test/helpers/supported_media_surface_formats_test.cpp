/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/helpers/supported_media_surface_formats.h"
#include "shared/source/helpers/surface_format_info.h"
#include "shared/test/common/test_macros/test.h"

std::pair<NEO::GFX3DSTATE_SURFACEFORMAT, bool> formatsVithSupportValue[171]{
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_SINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R64G64_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32X32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_SINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32B32_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_SINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_SINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_FLOAT_X8X24_TYPELESS, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_X32_TYPELESS_G8X24_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L32A32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R64_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16X16_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16X16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A32X32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L32X32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_I32X32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32G32_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B8G8R8A8_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B8G8R8A8_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10_SNORM_A2_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_SNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_SINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_SNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_SINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_FLOAT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B10G10R10A2_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B10G10R10A2_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R11G11B10_FLOAT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_SINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_FLOAT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R24_UNORM_X8_TYPELESS, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_X24_TYPELESS_G8_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L16A16_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_I24X8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L24X8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A24X8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_I32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A32_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B8G8R8X8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B8G8R8X8_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8X8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8X8_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R9G9B9E5_SHAREDEXP, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B10G10R10X2_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L16A16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10X2_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R32_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B5G6R5_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B5G6R5_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B5G5R5A1_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B5G5R5A1_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B4G4R4A4_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B4G4R4A4_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8_SNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8_SINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_SNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_SINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_FLOAT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_I16_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L16_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A16_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L8A8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_I16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L8A8_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R5G5_SNORM_B6_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B5G5R5X1_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_B5G5R5X1_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8_SNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8_SINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8_UINT, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A8_UNORM, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_I8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_P4A4_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_A4P4_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_P8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_L8_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_Y8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_DXT1_RGB_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R1_UINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_YCRCB_NORMAL, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUVY, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_P2_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC1_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC2_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC3_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC4_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC5_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC1_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC2_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC3_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_MONO8, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUV, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPY, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_DXT1_RGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_FXT1, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R8G8B8_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R64G64B64A64_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R64G64B64_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC4_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC5_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16_FLOAT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16_SSCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R16G16B16_USCALED, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC6H_SF16, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC7_UNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC7_UNORM_SRGB, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_BC6H_UF16, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_PLANAR_420_16, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_PACKED_422_16, true),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_SINT, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_SNORM, false),
    std::make_pair(NEO::GFX3DSTATE_SURFACEFORMAT_RAW, false)};

using MediaBlockSupportedFormatsTests = ::testing::TestWithParam<std::pair<NEO::GFX3DSTATE_SURFACEFORMAT, bool>>;

INSTANTIATE_TEST_CASE_P(AllFormatsWithMediaBlockSupportValue,
                        MediaBlockSupportedFormatsTests,
                        ::testing::ValuesIn(formatsVithSupportValue));

TEST_P(MediaBlockSupportedFormatsTests, givenSurfaceFormatWhenIsMediaFormatSupportedThenCorrectValueReturned) {
    EXPECT_EQ(NEO::SupportedMediaFormatsHelper::isMediaFormatSupported(GetParam().first), GetParam().second);
}
