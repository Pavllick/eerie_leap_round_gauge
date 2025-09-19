#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "main_view.h"

namespace eerie_leap::views {

LOG_MODULE_REGISTER(main_view_logger);

MainView::MainView() {}

void MainView::AddScreen(uint32_t id, std::unique_ptr<IScreen> screen) {
    screens_[id] = std::move(screen);
}

int MainView::Render() {
    RenderBackground();

    for(auto& screen : screens_)
        screen.second->Render();

    return 0;
}

void MainView::RenderBackground() {
    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_refr_now(lv_display_get_default());
}

} // namespace eerie_leap::views
