#pragma once

namespace eerie_leap::domain::ui_domain::event_bus {

enum class UiEventType : uint32_t {
    None = 0,
    SensorDataUpdated,
    StatusUpdated,
    LoggingStatusUpdated
};

} // namespace eerie_leap::domain::ui_domain::event_bus
