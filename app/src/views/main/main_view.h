#pragma once

#include <lvgl.h>

namespace eerie_leap::views::main {

struct LvIndicatorObjects {
    int32_t range_start;
    int32_t range_end;
    int32_t value;
    lv_obj_t* ui_arc;
    lv_obj_t* ui_label;
};

class MainView {
private:
    lv_anim_t value_change_animation_;
    LvIndicatorObjects indicator_objects_;

    lv_obj_t* CreateArcIndicator(int32_t range_start, int32_t range_end);
    lv_obj_t* CreateLabelIndicator();
    lv_anim_t CreateValueChangeAnimation();
    static void UpdateIndicator(void* obj, int32_t value);

    void ValueChangeAnimation(lv_anim_t anim, int32_t range, int32_t start_value, int32_t end_value);

public:
    MainView();

    int Render();
    void Update(int32_t value);
};

} // namespace eerie_leap::views::main
