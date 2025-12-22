#pragma once

#include <memory>
#include <memory_resource>
#include <string>
#include <vector>
#include <unordered_map>

#include "screen_configuration.h"

namespace eerie_leap::domain::ui_domain::models {

using namespace eerie_leap::domain::ui_domain::models;

struct UiConfiguration {
    using allocator_type = std::pmr::polymorphic_allocator<>;

    uint32_t active_screen_index;
    std::pmr::unordered_map<std::pmr::string, ConfigValue> properties;
    std::pmr::vector<std::shared_ptr<ScreenConfiguration>> screen_configurations;

    UiConfiguration(std::allocator_arg_t, allocator_type alloc)
        : properties(alloc),
          screen_configurations(alloc) {}

    UiConfiguration(const UiConfiguration&) = delete;
    UiConfiguration& operator=(const UiConfiguration&) = delete;
    UiConfiguration& operator=(UiConfiguration&&) noexcept = default;
    UiConfiguration(UiConfiguration&&) noexcept = default;
    ~UiConfiguration() = default;

    UiConfiguration(UiConfiguration&& other, allocator_type alloc)
        : active_screen_index(other.active_screen_index),
          properties(std::move(other.properties), alloc),
          screen_configurations(std::move(other.screen_configurations), alloc) {}
};

} // namespace eerie_leap::domain::ui_domain::models
