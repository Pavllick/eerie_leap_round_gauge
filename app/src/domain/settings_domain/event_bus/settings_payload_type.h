#pragma once

#include <cstdint>

namespace eerie_leap::domain::settings_domain::event_bus {

enum class SettingsPayloadType : std::uint32_t {
    SettingId
};

} // namespace eerie_leap::domain::settings_domain::event_bus
