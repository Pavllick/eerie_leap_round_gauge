#pragma once

#include <memory_resource>
#include <string>
#include <unordered_map>

#include "utilities/type/config_value.h"

#include "widget_type.h"
#include "widget_position.h"
#include "widget_size.h"

namespace eerie_leap::domain::ui_domain::models {

using namespace eerie_leap::utilities::type;

struct WidgetConfiguration {
    using allocator_type = std::pmr::polymorphic_allocator<>;

    WidgetType type;
    uint32_t id;
    WidgetPosition position_grid;
    WidgetSize size_grid;
    std::pmr::unordered_map<std::pmr::string, ConfigValue> properties;

    WidgetConfiguration(std::allocator_arg_t, allocator_type alloc)
        : properties(alloc) {}

    WidgetConfiguration(const WidgetConfiguration&) = delete;
    WidgetConfiguration& operator=(const WidgetConfiguration&) = delete;
    WidgetConfiguration& operator=(WidgetConfiguration&&) noexcept = default;
    WidgetConfiguration(WidgetConfiguration&&) noexcept = default;
    ~WidgetConfiguration() = default;

    WidgetConfiguration(WidgetConfiguration&& other, allocator_type alloc)
        : type(other.type),
          id(other.id),
          position_grid(other.position_grid),
          size_grid(other.size_grid),
          properties(std::move(other.properties), alloc) {}
};

} // namespace eerie_leap::domain::ui_domain::models
