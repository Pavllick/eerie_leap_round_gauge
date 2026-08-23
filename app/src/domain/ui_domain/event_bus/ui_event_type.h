#pragma once

namespace eerie_leap::domain::ui_domain::event_bus {

// Persisted as an int in widget configuration (UI_EVENT_TYPE) - append only.
enum class UiEventType : uint32_t {
    None = 0,
    SensorDataUpdated,
    StatusUpdated,
    LoggingStatusUpdated,
    NavigationChanged,
    SettingChanged
};

} // namespace eerie_leap::domain::ui_domain::event_bus
