#include "arc_fill_indicator.h"

namespace eerie_leap::views::widgets::indicators {

ArcFillIndicator::ArcFillIndicator(float range_start, float range_end) : state_() {
    state_.range_start = range_start;
    state_.range_end = range_end;
    state_.value = range_start;
}

int ArcFillIndicator::Render() {
    state_.lv_obj = CreateArcIndicator(static_cast<int32_t>(state_.range_start), static_cast<int32_t>(state_.range_end));

    return 0;
}

lv_obj_t* ArcFillIndicator::CreateArcIndicator(int32_t range_start, int32_t range_end) {
    lv_obj_t* ui_arc = lv_arc_create(lv_screen_active());
    lv_arc_set_range(ui_arc, range_start, range_end);
    lv_obj_set_width(ui_arc, lv_pct(100));
    lv_obj_set_height(ui_arc, lv_pct(100));
    lv_obj_set_align(ui_arc, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_arc, LV_OBJ_FLAG_CLICKABLE);
    lv_arc_set_value(ui_arc, range_start);
    lv_obj_set_style_arc_opa(ui_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_arc, lv_color_hex(0x1BBE5F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    return ui_arc;
}

void ArcFillIndicator::Update(float value) {
    lv_arc_set_value(state_.lv_obj, static_cast<int32_t>(value));
    state_.value = value;
}

} // namespace eerie_leap::views::widgets::indicators
