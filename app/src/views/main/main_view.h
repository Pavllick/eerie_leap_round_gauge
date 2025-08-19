#pragma once

#include <lvgl.h>

namespace eerie_leap::views::main {

class MainView {
private:
    lv_obj_t* ui_label_;
    lv_obj_t* ui_arc_;

    lv_obj_t* CreateArcIndicator();
    lv_obj_t* CreateLabelIndicator();

public:
    MainView() {}

    int Render();
    void Update(int32_t value);
};

} // namespace eerie_leap::views::main
