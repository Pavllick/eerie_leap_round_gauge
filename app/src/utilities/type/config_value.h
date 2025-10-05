#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <type_traits>

namespace eerie_leap::utilities::type {

using ConfigValue = std::variant<
    std::monostate,
    int,
    double,
    std::string,
    bool,
    std::vector<int>,
    std::vector<std::string>,
    std::unordered_map<std::string, std::string>
>;

template<typename T>
T GetConfigValue(
    const std::unordered_map<std::string, ConfigValue>& config,
    const std::string& key,
    const T& default_value) {

    auto it = config.find(key);

    if(it == config.end()) {
        return default_value;
    }

    const ConfigValue& value = it->second;

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
