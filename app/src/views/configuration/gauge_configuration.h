#pragma once

#include <vector>
#include <unordered_map>

#include "views/screens/configuration/screen_configuration.h"

namespace eerie_leap::views::configuration {

using namespace eerie_leap::views::screens::configuration;

struct GaugeConfiguration {
    uint32_t active_screen_index;
    std::unordered_map<std::string, ConfigValue> properties;
    std::vector<ScreenConfiguration> screen_configurations;
};

} // namespace eerie_leap::views::configuration
