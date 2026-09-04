#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <type_traits>
#include <memory_resource>

namespace eerie_leap::utilities::type {

using ConfigValue = std::variant<
    std::monostate,
    int,
    double,
    std::pmr::string,
    bool,
    std::pmr::vector<int>,
    std::pmr::vector<std::pmr::string>,
    std::pmr::unordered_map<std::pmr::string, std::pmr::string>
>;

template<typename T>
T ConfigValueAs(const ConfigValue& value, const T& default_value) {
    if(std::holds_alternative<T>(value)) {
        return std::get<T>(value);
    }

    if constexpr (std::is_same_v<T, double>) {
        if (std::holds_alternative<int>(value)) {
            return static_cast<double>(std::get<int>(value));
        }
    }
    if constexpr (std::is_same_v<T, int>) {
        if (std::holds_alternative<double>(value)) {
            return static_cast<int>(std::get<double>(value));
        }
    }

    return default_value;
}

} // namespace eerie_leap::utilities::type
