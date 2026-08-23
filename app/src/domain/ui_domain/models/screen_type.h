#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

// Persisted in the screen configuration - append only.
enum class ScreenType : uint32_t {
    None = 0,
    System,
    Gauge,
    Settings,
    Popup
};

} // namespace eerie_leap::domain::ui_domain::models
