#pragma once

#include <memory>

#include <lvgl.h>

#include "views/widgets/utilitites/frame.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::views::widgets::utilitites;

struct WidgetState {
    std::shared_ptr<Frame> container;
    lv_obj_t* lv_obj;
};

} // namespace eerie_leap::views::widgets
