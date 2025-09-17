#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/display.h>
#include <lvgl_mem.h>

#include "ui_renderer.h"

namespace eerie_leap::domain::ui_domain {

LOG_MODULE_REGISTER(renderer_logger);

const device* UiRenderer::display_dev_ = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// Should be allocated on internal RAM to take an advantage of DMA for rendering
K_KERNEL_STACK_MEMBER(UiRenderer::stack_area_, UiRenderer::k_stack_size_);

int UiRenderer::Initialize() {
    if(!device_is_ready(display_dev_)) {
		LOG_ERR("Device not ready, aborting test");
		return -1;
	}

    auto* act_scr = lv_display_get_default();
    lv_display_add_event_cb(act_scr, DisplayInvalidateCb, LV_EVENT_INVALIDATE_AREA, nullptr);

    display_blanking_off(display_dev_);
    display_set_brightness(display_dev_, 160);

    return 0;
}

k_tid_t UiRenderer::Start() {
    running_ = true;

    thread_id_ = k_thread_create(
        &thread_data_,
        stack_area_,
        K_THREAD_STACK_SIZEOF(stack_area_),
        [](void* instance, void* p2, void* p3) { static_cast<UiRenderer*>(instance)->UiRendererThreadEntry(); },
        this, nullptr, nullptr,
        k_priority_, 0, K_NO_WAIT);

    k_thread_name_set(&thread_data_, "ui_renderer");

    LOG_INF("UI renderer started.");

    return thread_id_;
}

void UiRenderer::Stop() {
    running_ = false;
    k_thread_join(thread_id_, K_MSEC(100));
}

void UiRenderer::UiRendererThreadEntry() {
    while(running_)
        Render();
}

void UiRenderer::Render() {
    uint32_t sleep_ms = lv_timer_handler();
    k_msleep(MIN(sleep_ms, INT32_MAX));
}

// NOTE: Fixes incorrect invalidation of redrawn area
// https://forum.lvgl.io/t/lvgl-9-2-esp32-s3-display-axs15321b/19735/5
void UiRenderer::DisplayInvalidateCb(lv_event_t* e) {
    auto* area = (lv_area_t*)lv_event_get_param(e);

    // Round down to even
    area->x1 &= ~1;
    area->y1 &= ~1;

    // Round up to odd
    area->x2 |= 1;
    area->y2 |= 1;
}

} // namespace eerie_leap::domain::ui_domain
