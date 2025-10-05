#pragma once

#include <memory>
#include "utilities/memory/heap_allocator.h"

namespace eerie_leap::configuration::services {

using namespace eerie_leap::utilities::memory;

template <typename T>
struct LoadedConfig {
public:
    ext_unique_ptr<ExtVector> config_raw;
    ext_unique_ptr<T> config;
};

}
