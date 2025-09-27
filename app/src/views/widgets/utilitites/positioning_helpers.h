#pragma once

#include <lvgl.h>

#include "domain/ui_domain/models/widget_position.h"

namespace eerie_leap::views::widgets::utilitites {

using namespace eerie_leap::domain::ui_domain::models;

class PositioningHelpers {
public:
    static WidgetPosition GetPointOnCircle(int center_x, int center_y, int radius_px, float angle_deg);
    static void PlaceObjectOnCircle(lv_obj_t* obj, int center_x, int center_y, int radius_px, float angle_deg);
};

} // namespace eerie_leap::views::widgets::utilitites
