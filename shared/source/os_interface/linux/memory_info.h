/*
 * Copyright (C) 2019-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/os_interface/linux/ioctl_helper.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace NEO {
class Drm;
struct HardwareInfo;

class MemoryInfo {
  public:
    using RegionContainer = std::vector<MemoryRegion>;

    virtual ~MemoryInfo(){};

    MemoryInfo(const RegionContainer &regionInfo, const Drm &drm);

    void assignRegionsFromDistances(const std::vector<DistanceInfo> &distances);

    MOCKABLE_VIRTUAL int createGemExt(const MemRegionsVec &memClassInstances, size_t allocSize, uint32_t &handle, uint64_t patIndex, std::optional<uint32_t> vmId, int32_t pairHandle, bool isChunked, uint32_t numOfChunks);

    MemoryClassInstance getMemoryRegionClassAndInstance(uint32_t memoryBank, const HardwareInfo &hwInfo);

    MOCKABLE_VIRTUAL size_t getMemoryRegionSize(uint32_t memoryBank);

    const MemoryRegion &getMemoryRegion(uint32_t memoryBank);

    void printRegionSizes();

    uint32_t getTileIndex(uint32_t memoryBank);

    MOCKABLE_VIRTUAL int createGemExtWithSingleRegion(uint32_t memoryBanks, size_t allocSize, uint32_t &handle, uint64_t patIndex, int32_t pairHandle);
    MOCKABLE_VIRTUAL int createGemExtWithMultipleRegions(uint32_t memoryBanks, size_t allocSize, uint32_t &handle, uint64_t patIndex);
    MOCKABLE_VIRTUAL int createGemExtWithMultipleRegions(uint32_t memoryBanks, size_t allocSize, uint32_t &handle, uint64_t patIndex, int32_t pairHandle, bool isChunked, uint32_t numOfChunks);

    const RegionContainer &getDrmRegionInfos() const { return drmQueryRegions; }

  protected:
    const Drm &drm;
    const RegionContainer drmQueryRegions;

    const MemoryRegion &systemMemoryRegion;

    RegionContainer localMemoryRegions;
};

} // namespace NEO
