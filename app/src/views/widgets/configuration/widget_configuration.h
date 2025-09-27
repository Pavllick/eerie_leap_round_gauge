#pragma once

#include <string>
#include <unordered_map>

#include "utilities/type/config_value.h"

#include "widget_type.h"
#include "widget_position.h"
#include "widget_size.h"

namespace eerie_leap::views::widgets::configuration {

using namespace eerie_leap::utilities::type;

struct WidgetConfiguration {
    WidgetType type;
    uint32_t id;
    WidgetPosition position_grid;
    WidgetSize size_grid;
    std::unordered_map<std::string, ConfigValue> properties;
};

} // namespace eerie_leap::views::widgets::configuration
