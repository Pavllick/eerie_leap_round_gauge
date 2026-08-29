#pragma once

#include <cstdint>

namespace eerie_leap::domain::settings_domain::event_bus {

enum class SettingsEventType : std::uint32_t {
    Changed
};

} // namespace eerie_leap::domain::settings_domain::event_bus
