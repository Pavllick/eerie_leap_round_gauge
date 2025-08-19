#include <string>

#include <zephyr/kernel.h>

#include "main_view.h"

namespace eerie_leap::views::main {

int MainView::Render() {
    ui_arc_ = CreateArcIndicator();
    ui_label_ = CreateLabelIndicator();

    return 0;
}

lv_obj_t* MainView::CreateArcIndicator() {
    lv_obj_t* ui_arc = lv_arc_create(lv_screen_active());
    lv_arc_set_range(ui_arc, 0, 99);
    lv_obj_set_width(ui_arc, lv_pct(100));
    lv_obj_set_height(ui_arc, lv_pct(100));
    lv_obj_set_align(ui_arc, LV_ALIGN_CENTER);
    lv_obj_remove_flag(ui_arc, LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_arc_set_value(ui_arc, 0);
    lv_obj_set_style_arc_opa(ui_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_arc, lv_color_hex(0x1BBE5F), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_arc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    // lv_anim_t anim;
    // lv_anim_init(&anim);
    // lv_anim_set_var(&anim, ui_arc);
    // lv_anim_set_exec_cb(&anim, update_label);
    // lv_anim_set_duration(&anim, 2000);
    // lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    // lv_anim_set_repeat_delay(&anim, 500);
    // lv_anim_set_values(&anim, 0, 99);
    // lv_anim_start(&anim);

    return ui_arc;
}

lv_obj_t* MainView::CreateLabelIndicator() {
    auto ui_label = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label, "0");
    lv_obj_set_style_text_font(ui_label, &lv_font_unscii_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_scale(ui_label, 1500, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_label, -65);
    lv_obj_set_y(ui_label, -55);

    return ui_label;
}

void MainView::Update(int32_t value) {
    lv_arc_set_value(ui_arc_, value);
    lv_label_set_text(ui_label_, std::to_string(value).c_str());
}

} // namespace eerie_leap::views::main
