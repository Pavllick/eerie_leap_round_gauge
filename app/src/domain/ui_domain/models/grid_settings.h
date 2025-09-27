#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

struct GridSettings {
    bool snap_enabled;
    uint32_t width;
    uint32_t height;
    uint32_t spacing_px;
};

} // namespace eerie_leap::domain::ui_domain::models
