#pragma once

#include <cstddef>

#include "event.h"
#include "event_filter.h"

namespace eerie_leap::subsys::event_bus {

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
struct Subscription {
    size_t id;
    EventTypeEnum event_type;
    std::function<bool(const Event<EventTypeEnum, PayloadTypeEnum>&)> filter;
    EventHandler<EventTypeEnum, PayloadTypeEnum> handler;

    template<EventFilter<EventTypeEnum, PayloadTypeEnum> FilterType>
    Subscription(size_t sub_id, EventTypeEnum type, FilterType f, EventHandler<EventTypeEnum, PayloadTypeEnum> h)
        : id(sub_id), event_type(type), filter(f), handler(std::move(h)) {}
};

} // namespace eerie_leap::subsys::event_bus
