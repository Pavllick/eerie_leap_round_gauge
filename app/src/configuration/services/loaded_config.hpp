#pragma once

#include <memory>
#include "utilities/memory/heap_allocator.h"

namespace eerie_leap::configuration::services {

using namespace eerie_leap::utilities::memory;

template <typename T>
struct LoadedConfig {
public:
    std::shared_ptr<ExtVector> config_raw;
    std::shared_ptr<T> config;
};

}
