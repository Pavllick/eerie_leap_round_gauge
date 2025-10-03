#pragma once

namespace eerie_leap::domain::ui_domain::event_bus {

enum class UiPayloadType : uint32_t {
    None = 0,
    Value,
    SensorId,
    IsStatusOk,
    ComUserStatus
};

} // namespace eerie_leap::domain::ui_domain::event_bus
