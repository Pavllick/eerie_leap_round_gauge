#pragma once

#include <cstdint>

namespace eerie_leap::domain::display_domain::models {

struct DisplayConfiguration {
    uint8_t brightness = 160;
    bool blanking_enabled = false;

    // 0 = never blank on inactivity.
    uint32_t screen_timeout_s = 0;
    uint32_t theme_id = 0;
};

} // namespace eerie_leap::domain::display_domain::models
