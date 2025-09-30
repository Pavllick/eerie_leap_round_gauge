#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "main_view.h"

namespace eerie_leap::views {

LOG_MODULE_REGISTER(main_view_logger);

MainView::MainView() {}

void MainView::AddScreen(uint32_t id, std::shared_ptr<IScreen> screen) {
    screens_[id] = std::move(screen);
}

int MainView::SetActiveScreen(uint32_t id) {
    if(screens_.find(id) == screens_.end())
        return -1;

    active_screen_id_ = id;
    return 0;
}

std::shared_ptr<IScreen> MainView::GetScreen(uint32_t id) {
    if(screens_.find(id) == screens_.end())
        return nullptr;

    return screens_.at(id);
}

std::shared_ptr<IScreen> MainView::GetActiveScreen() {
    return GetScreen(active_screen_id_);
}

int MainView::DoRender() {
    RenderBackground();

    for(auto& screen : screens_)
        screen.second->Render();

    return 0;
}

void RenderCenterCrossHelperGuides(lv_obj_t* screen) {
    lv_obj_t * panel1 = lv_obj_create(screen);
    lv_obj_set_width(panel1, 2);
    lv_obj_set_height(panel1, 466);
    lv_obj_set_align(panel1, LV_ALIGN_CENTER);
    lv_obj_remove_flag(panel1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(panel1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * panel2 = lv_obj_create(screen);
    lv_obj_set_width(panel2, 466);
    lv_obj_set_height(panel2, 2);
    lv_obj_set_align(panel2, LV_ALIGN_CENTER);
    lv_obj_remove_flag(panel2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(panel2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void MainView::RenderBackground() {
    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // RenderCenterCrossHelperGuides(screen);

    lv_refr_now(lv_display_get_default());
}

} // namespace eerie_leap::views
