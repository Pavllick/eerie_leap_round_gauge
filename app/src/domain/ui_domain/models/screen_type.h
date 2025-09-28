#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

enum class ScreenType : uint32_t {
    None = 0,
    System,
    Gauge
};

} // namespace eerie_leap::domain::ui_domain::models
