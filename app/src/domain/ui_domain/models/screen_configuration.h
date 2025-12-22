#pragma once

#include <memory>
#include <memory_resource>
#include <vector>

#include "screen_type.h"
#include "widget_configuration.h"
#include "grid_settings.h"

namespace eerie_leap::domain::ui_domain::models {

struct ScreenConfiguration {
    using allocator_type = std::pmr::polymorphic_allocator<>;

    uint32_t id;
    ScreenType type;
    GridSettings grid;
    std::pmr::vector<std::shared_ptr<WidgetConfiguration>> widget_configurations;

    ScreenConfiguration(std::allocator_arg_t, allocator_type alloc)
        : widget_configurations(alloc) {}

    ScreenConfiguration(const ScreenConfiguration&) = delete;
    ScreenConfiguration& operator=(const ScreenConfiguration&) = delete;
    ScreenConfiguration& operator=(ScreenConfiguration&&) noexcept = default;
    ScreenConfiguration(ScreenConfiguration&&) noexcept = default;
    ~ScreenConfiguration() = default;

    ScreenConfiguration(ScreenConfiguration&& other, allocator_type alloc)
        : id(other.id),
          type(other.type),
          grid(other.grid),
          widget_configurations(std::move(other.widget_configurations), alloc) {}
};

} // namespace eerie_leap::domain::ui_domain::models
