#pragma once

namespace eerie_leap::domain::ui_domain::event_bus {

enum class UiPayloadType : uint32_t {
    VALUE,
    SENSOR_ID,
    IS_STATUS_OK,
    COM_USER_STATUS
};

} // namespace eerie_leap::domain::ui_domain::event_bus
