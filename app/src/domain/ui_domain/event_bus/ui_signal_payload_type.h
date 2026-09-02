#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::event_bus {

// A signal carries exactly one value; the signal itself is the event type.
enum class UiSignalPayloadType : std::uint32_t {
    Value
};

} // namespace eerie_leap::domain::ui_domain::event_bus
