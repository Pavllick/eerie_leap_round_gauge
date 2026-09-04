#pragma once

#include <variant>

#include "subsys/event_bus/event.h"

#include "utilities/type/config_value.h"

namespace eerie_leap::views::widgets {

using eerie_leap::subsys::event_bus::EventData;
using eerie_leap::utilities::type::ConfigValue;

// EventData and ConfigValue overlap but are not the same variant: the bus has no monostate and
// carries uint32/float, the configuration model carries double and PMR strings.
inline ConfigValue ToConfigValue(const EventData& value) {
    return std::visit([](auto&& argument) -> ConfigValue {
        using T = std::decay_t<decltype(argument)>;

        if constexpr (std::is_same_v<T, int>)
            return static_cast<int>(argument);
        else if constexpr (std::is_same_v<T, uint32_t>)
            return static_cast<int>(argument);
        else if constexpr (std::is_same_v<T, float>)
            return static_cast<double>(argument);
        else if constexpr (std::is_same_v<T, bool>)
            return argument;
        else
            return std::pmr::string(argument.c_str());
    }, value);
}

inline EventData ToEventData(const ConfigValue& value) {
    return std::visit([](auto&& argument) -> EventData {
        using T = std::decay_t<decltype(argument)>;

        if constexpr (std::is_same_v<T, int>)
            return argument;
        else if constexpr (std::is_same_v<T, double>)
            return static_cast<float>(argument);
        else if constexpr (std::is_same_v<T, bool>)
            return argument;
        else if constexpr (std::is_same_v<T, std::pmr::string>)
            return std::string(argument.c_str());
        else
            return 0;
    }, value);
}

} // namespace eerie_leap::views::widgets
