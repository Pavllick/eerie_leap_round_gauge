#pragma once

namespace eerie_leap::domain::ui_domain::event_bus {

enum class UiPayloadType : uint32_t {
    None = 0,
    Value,
    SensorId,
    IsStatusOk,
    ComUserStatus,
    NavigationAction,
    TargetGroupId,
    TargetScreenId,
    SettingId
};

} // namespace eerie_leap::domain::ui_domain::event_bus
