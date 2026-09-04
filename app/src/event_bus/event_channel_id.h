#pragma once

#include <cstdint>

namespace eerie_leap::event_bus {

// Persisted as the channel a widget property binding reads from - append only.
enum class EventChannelId : uint32_t {
    None = 0,
    Sensors,
    Logging,
    Navigation,
    Settings,
};

} // namespace eerie_leap::event_bus
