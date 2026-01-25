#include <string>

#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "views/themes/theme_manager.h"

#include "main_view.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::themes;

LOG_MODULE_REGISTER(main_view_logger);

MainView::MainView() {
    container_ = std::make_shared<Frame>(Frame::CreateWrapped()
        .SetWidth(100, false)
        .SetHeight(100, false)
        .Build());
}

std::shared_ptr<Frame> MainView::GetContainer() const {
    return container_;
}

void MainView::AddScreen(uint32_t id, std::shared_ptr<IScreen> screen) {
    screens_[id] = std::move(screen);
}

int MainView::SetActiveScreen(uint32_t id) {
    if(!screens_.contains(id))
        return -1;

    active_screen_id_ = id;
    return 0;
}

std::shared_ptr<IScreen> MainView::GetScreen(uint32_t id) {
    if(!screens_.contains(id))
        return nullptr;

    return screens_.at(id);
}

std::shared_ptr<IScreen> MainView::GetActiveScreen() {
    return GetScreen(active_screen_id_);
}

void RenderCenterCrossHelperGuides(lv_obj_t* screen) {
    lv_obj_t * panel1 = lv_obj_create(screen);
    lv_obj_set_width(panel1, 2);
    lv_obj_set_height(panel1, 466);
    lv_obj_set_align(panel1, LV_ALIGN_CENTER);
    lv_obj_remove_flag(panel1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(
        panel1,
        ThemeManager::GetInstance().GetCurrentTheme().GetPrimaryColor().ToLvColor(),
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * panel2 = lv_obj_create(screen);
    lv_obj_set_width(panel2, 466);
    lv_obj_set_height(panel2, 2);
    lv_obj_set_align(panel2, LV_ALIGN_CENTER);
    lv_obj_remove_flag(panel2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(
        panel2,
        ThemeManager::GetInstance().GetCurrentTheme().GetPrimaryColor().ToLvColor(),
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

int MainView::DoRender() {
    for(auto& screen : screens_)
        screen.second->Render();

    // RenderCenterCrossHelperGuides(container_->GetObject());

    return 0;
}

int MainView::ApplyTheme(const ITheme& theme) {
    lv_obj_set_style_bg_color(container_->GetObject(), theme.GetBackgroundColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container_->GetObject(), theme.GetBackgroundColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

} // namespace eerie_leap::views
