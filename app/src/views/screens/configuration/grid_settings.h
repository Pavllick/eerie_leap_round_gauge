#pragma once

#include <cstdint>

namespace eerie_leap::views::screens::configuration {

struct GridSettings {
    bool snap_enabled;
    uint32_t grid_width;
    uint32_t grid_height;
    uint32_t spacing_px;
};

} // namespace eerie_leap::views::screens::configuration
