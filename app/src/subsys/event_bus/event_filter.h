#pragma once

#include <concepts>

#include "event.h"

namespace eerie_leap::subsys::event_bus {

template<typename T, typename EventTypeEnum, typename PayloadTypeEnum>
concept EventFilter = requires(T filter, const Event<EventTypeEnum, PayloadTypeEnum>& event) {
    { filter(event) } -> std::convertible_to<bool>;
};

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
struct AcceptAllFilter {
    bool operator()(const Event<EventTypeEnum, PayloadTypeEnum>&) const { return true; }
};

} // namespace eerie_leap::subsys::event_bus
