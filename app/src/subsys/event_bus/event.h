#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <variant>
#include <string>
#include <unordered_map>
#include <functional>

#include "utilities/concepts/concepts.h"

namespace eerie_leap::subsys::event_bus {

using namespace eerie_leap::utilities::concepts;

using EventData = std::variant<int, uint32_t, float, std::string, bool>;

template<EnumClassUint32 PayloadTypeEnum>
using EventPayload = std::unordered_map<PayloadTypeEnum, EventData>;

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
struct Event {
    EventTypeEnum type;
    EventPayload<PayloadTypeEnum> payload;
    std::string source_id;
};

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
using EventHandler = std::function<void(const Event<EventTypeEnum, PayloadTypeEnum>&)>;

} // namespace eerie_leap::subsys::event_bus
