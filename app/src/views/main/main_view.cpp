#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "main_view.h"

namespace eerie_leap::views::main {

LOG_MODULE_REGISTER(main_view_logger);

MainView::MainView() : indicator_objects_() {}

int MainView::Render() {
    indicator_objects_.ui_arc = CreateArcIndicator(0, 100);
    indicator_objects_.ui_label = CreateLabelIndicator();
    value_change_animation_ = CreateValueChangeAnimation();

    return 0;
}

lv_obj_t* MainView::CreateArcIndicator(int32_t range_start, int32_t range_end) {
    lv_obj_t* ui_arc = lv_arc_create(lv_screen_active());
    lv_arc_set_range(ui_arc, range_start, range_end);
    lv_obj_set_width(ui_arc, lv_pct(100));
    lv_obj_set_height(ui_arc, lv_pct(100));
    lv_obj_set_align(ui_arc, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_arc, LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_arc_set_value(ui_arc, range_start);
    lv_obj_set_style_arc_opa(ui_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_arc, lv_color_hex(0x1BBE5F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    return ui_arc;
}

lv_obj_t* MainView::CreateLabelIndicator() {
    auto ui_label = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_label, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label, "0");
    lv_obj_set_style_text_font(ui_label, &lv_font_unscii_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_scale(ui_label, 1500, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_label, -65);
    lv_obj_set_y(ui_label, -55);

    return ui_label;
}

void MainView::UpdateIndicator(void* obj, int32_t value) {
    auto* indicator_objects = (LvIndicatorObjects*)obj;
    lv_obj_t* ui_arc = indicator_objects->ui_arc;
    lv_obj_t* ui_label = indicator_objects->ui_label;

    lv_arc_set_value(ui_arc, value);
    lv_label_set_text(ui_label, std::to_string(value).c_str());
}

lv_anim_t MainView::CreateValueChangeAnimation() {
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, &indicator_objects_);
    lv_anim_set_exec_cb(&anim, UpdateIndicator);
    lv_anim_set_duration(&anim, 1000);
    lv_anim_set_repeat_count(&anim, 0);

    return anim;
}

void MainView::ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value) {
    uint32_t duration = 4000;
    uint32_t unit_duration = duration / range;
    uint32_t change_duration = unit_duration * abs(end_value - start_value);

    lv_anim_set_duration(&anim, change_duration);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_start(&anim);
}

void MainView::Update(int32_t value) {
    // ValueChangeAnimation(value_change_animation_, 100, lv_arc_get_value(indicator_objects_.ui_arc), value);

    UpdateIndicator(&indicator_objects_, value);
}

} // namespace eerie_leap::views::main
