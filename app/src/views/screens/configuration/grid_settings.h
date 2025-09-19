#pragma once

#include <cstdint>

namespace eerie_leap::views::screens::configuration {

struct GridSettings {
    bool snap_enabled;
    uint32_t grid_size;
    uint32_t spacing;
};

} // namespace eerie_leap::views::screens::configuration
