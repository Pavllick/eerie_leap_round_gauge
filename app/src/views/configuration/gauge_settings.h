#pragma once

#include <unordered_map>

#include "utilities/type/config_value.h"

namespace eerie_leap::views::configuration {

using namespace eerie_leap::utilities::type;

struct GaugeSettings {
    std::unordered_map<std::string, ConfigValue> properties;
};

} // namespace eerie_leap::views::configuration
