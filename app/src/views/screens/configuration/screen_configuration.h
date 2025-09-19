#pragma once

#include <vector>

#include "views/widgets/configuration/widget_configuration.h"
#include "views/screens/configuration/grid_settings.h"

namespace eerie_leap::views::screens::configuration {

using namespace eerie_leap::views::widgets::configuration;

struct ScreenConfiguration {
    uint32_t id;
    GridSettings grid;
    std::vector<WidgetConfiguration> widget_configurations;
};

} // namespace eerie_leap::views::screens::configuration
