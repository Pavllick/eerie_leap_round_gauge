#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::event_bus {

enum class NavigationEventType : std::uint32_t {
    Changed
};

} // namespace eerie_leap::domain::ui_domain::event_bus
