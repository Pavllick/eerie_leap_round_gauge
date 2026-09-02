#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::event_bus {

// The vocabulary a widget binds to, persisted as an int in widget configuration
// (UI_SIGNAL_TYPE) - append only. UiSignalBridge maps domain events onto it.
enum class UiSignalType : std::uint32_t {
    None = 0,
    LoggingActive = 1
};

} // namespace eerie_leap::domain::ui_domain::event_bus
