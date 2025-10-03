#pragma once

#include <cstdint>
#include <lvgl.h>

namespace eerie_leap::domain::ui_domain::models {

enum class IconType : std::uint32_t {
    None = 0,
    Dot,
    Label,
    Svg
};

} // namespace eerie_leap::domain::ui_domain::models
