#pragma once

#include <cstdint>

namespace eerie_leap::domain::settings_domain::event_bus {

// Persisted as the payload key of a widget binding - append only.
enum class SettingsPayloadType : std::uint32_t {
    SettingId,
    Value,
    MinValue,
    MaxValue,
    Step
};

} // namespace eerie_leap::domain::settings_domain::event_bus
