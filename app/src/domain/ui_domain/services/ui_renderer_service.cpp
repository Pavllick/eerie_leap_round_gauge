#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <lvgl_mem.h>

#include "subsys/device_tree/dt_display.h"
#include "domain/ui_domain/lvgl_lock.h"

#include "ui_renderer_service.h"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::subsys::device_tree;

LOG_MODULE_REGISTER(renderer_logger);

UiRendererService::UiRendererService() {
    // NOTE: Stack should be allocated on internal RAM to take an advantage of DMA for rendering
    thread_ = std::make_unique<Thread>(
        "ui_renderer_service",
        this,
        UiRendererService::k_stack_size_,
        UiRendererService::k_priority_,
        false);
}

UiRendererService::~UiRendererService() {
    Stop();
}

int UiRendererService::Initialize() {
    if(DtDisplay::Get() == nullptr) {
        LOG_ERR("Display not found, aborting");
        return -1;
    }

    if(!device_is_ready(DtDisplay::Get())) {
		LOG_ERR("Display not ready, aborting");
		return -1;
	}

    thread_->Initialize();

    auto* act_scr = lv_display_get_default();
    lv_display_add_event_cb(act_scr, DisplayInvalidateCb, LV_EVENT_INVALIDATE_AREA, nullptr);

    lv_obj_t* screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

void UiRendererService::Start() {
    running_ = true;
    thread_->Start();

    LOG_INF("UI renderer service started.");
}

void UiRendererService::Stop() {
    running_ = false;
    thread_->Join();
}

void UiRendererService::ThreadEntry() {
    while(running_)
        Render();
}

void UiRendererService::Render() {
    LvglLock::GetInstance().Lock();
    uint32_t sleep_ms = lv_timer_handler();
    LvglLock::GetInstance().Unlock();

    k_msleep(MIN(sleep_ms, INT32_MAX));
}

// NOTE: Fixes incorrect invalidation of redrawn area
// https://forum.lvgl.io/t/lvgl-9-2-esp32-s3-display-axs15321b/19735/5
void UiRendererService::DisplayInvalidateCb(lv_event_t* e) {
    auto* area = (lv_area_t*)lv_event_get_param(e);

    // Round down to even
    area->x1 &= ~1;
    area->y1 &= ~1;

    // Round up to odd
    area->x2 |= 1;
    area->y2 |= 1;
}

} // namespace eerie_leap::domain::ui_domain::services
