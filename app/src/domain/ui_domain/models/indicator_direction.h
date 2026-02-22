#pragma once

#include <cstdint>
#include <lvgl.h>

namespace eerie_leap::domain::ui_domain::models {

enum class InidicatorDirection : uint8_t {
    None = 0,
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

} // namespace eerie_leap::domain::ui_domain::models
