#include <cmath>
#include <numbers>

#include "positioning_helpers.h"

namespace eerie_leap::views::utilitites {

WidgetPosition PositioningHelpers::GetPointOnCircle(int center_x, int center_y, int radius_px, float angle_deg) {
    float angle_radians = angle_deg * std::numbers::pi / 180.0f;
    int x = center_x + (radius_px * cos(angle_radians));
    int y = center_y + (radius_px * sin(angle_radians));

    return {x, y};
}

void PositioningHelpers::PlaceObjectOnCircle(lv_obj_t* obj, int center_x, int center_y, int radius_px, float angle_deg) {
    WidgetPosition pos = GetPointOnCircle(center_x, center_y, radius_px, angle_deg);

    int angle = 2700 - (3600 - static_cast<int>(angle_deg * 10));

    lv_obj_set_style_transform_angle(obj, angle, 0);
    lv_obj_set_x(obj, pos.x);
    lv_obj_set_y(obj, pos.y);
}

} // namespace eerie_leap::views::utilitites
