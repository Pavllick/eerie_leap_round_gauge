#pragma once

#include <vector>

#include "widget_configuration.h"
#include "domain/ui_domain/models/grid_settings.h"

namespace eerie_leap::domain::ui_domain::models {

struct ScreenConfiguration {
    uint32_t id;
    GridSettings grid;
    std::vector<WidgetConfiguration> widget_configurations;
};

} // namespace eerie_leap::domain::ui_domain::models
