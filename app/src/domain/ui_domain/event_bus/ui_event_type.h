#pragma once

namespace eerie_leap::domain::ui_domain::event_bus {

enum class UiEventType : uint32_t {
    NONE = 0,
    SENSOR_DATA_UPDATED,
    STATUS_UPDATED,
    LOGGING_STATUS_UPDATED
};

} // namespace eerie_leap::domain::ui_domain::event_bus
