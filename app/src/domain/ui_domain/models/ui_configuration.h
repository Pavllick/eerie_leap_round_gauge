#pragma once

#include <vector>
#include <unordered_map>

#include "screen_configuration.h"

namespace eerie_leap::domain::ui_domain::models {

using namespace eerie_leap::domain::ui_domain::models;

struct UiConfiguration {
    uint32_t active_screen_index;
    std::unordered_map<std::string, ConfigValue> properties;
    std::vector<ScreenConfiguration> screen_configurations;
};

} // namespace eerie_leap::domain::ui_domain::models
