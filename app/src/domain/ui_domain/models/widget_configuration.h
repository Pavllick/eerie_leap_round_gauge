#pragma once

#include <memory_resource>
#include <string>
#include <unordered_map>

#include "utilities/type/config_value.h"

#include "widget_type.h"
#include "widget_position.h"
#include "widget_size.h"

namespace eerie_leap::domain::ui_domain::models {

using eerie_leap::utilities::type::ConfigValue;

struct WidgetConfiguration {
    using allocator_type = std::pmr::polymorphic_allocator<>;

    WidgetType type;
    uint32_t id;
    WidgetPosition position_grid;
    WidgetSize size_grid;
    int32_t z_index = 0;
    bool is_visible = true;
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
          z_index(other.z_index),
          is_visible(other.is_visible),
          properties(std::move(other.properties), alloc) {}
};

} // namespace eerie_leap::domain::ui_domain::models
