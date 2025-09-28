#pragma once

#include <vector>

#include "screen_type.h"
#include "widget_configuration.h"
#include "grid_settings.h"

namespace eerie_leap::domain::ui_domain::models {

struct ScreenConfiguration {
    uint32_t id;
    ScreenType type;
    GridSettings grid;
    std::vector<WidgetConfiguration> widget_configurations;
};

} // namespace eerie_leap::domain::ui_domain::models
