#pragma once

#include <memory>
#include "utilities/memory/memory_resource_manager.h"

namespace eerie_leap::configuration::services {

using namespace eerie_leap::utilities::memory;

template <typename T>
struct LoadedConfig {
public:
    std::pmr::vector<uint8_t> config_raw;
    pmr_unique_ptr<T> config;
    uint32_t checksum;
};

}
