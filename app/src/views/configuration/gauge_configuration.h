#pragma once

#include <vector>

#include "views/screens/configuration/screen_configuration.h"

#include "gauge_settings.h"

namespace eerie_leap::views::configuration {

using namespace eerie_leap::views::screens::configuration;

struct GaugeConfiguration {
    int active_screen_index;
    GaugeSettings settings;
    std::vector<ScreenConfiguration> screen_configurations;
};

} // namespace eerie_leap::views::configuration
