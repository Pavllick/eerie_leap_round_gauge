#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::event_bus {

enum class NavigationPayloadType : std::uint32_t {
    Action,
    TargetGroupId,
    TargetScreenId
};

} // namespace eerie_leap::domain::ui_domain::event_bus
