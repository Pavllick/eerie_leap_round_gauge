#include <memory>
#include <ctime>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>

#include "utilities/memory/heap_allocator.h"
#include "utilities/dev_tools/system_info.h"
// #include "utilities/time/boot_elapsed_time_service.h"
// #include "domain/fs_domain/services/fs_service.h"

// #include "configuration/system_config/system_config.h"
// #include "configuration/services/configuration_service.h"
// #include "controllers/system_configuration_controller.h"

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
// using namespace eerie_leap::utilities::time;

// using namespace eerie_leap::controllers;

// using namespace eerie_leap::domain::fs_domain::services;
// using namespace eerie_leap::configuration::services;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 2000;
const size_t DISPLAY_WIDTH = 466;
const size_t DISPLAY_HEIGHT = 466;

static const device* const display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// NOTE: Fixes incorrect invalidation of redrawn area
void display_invalidate_cb(lv_event_t *e) {
    auto *area = (lv_area_t *)lv_event_get_param(e);

    uint16_t x1 = area->x1;
    uint16_t x2 = area->x2;

    uint16_t y1 = area->y1;
    uint16_t y2 = area->y2;

    // round the start of coordinate down to the nearest 2M number
    area->x1 = (x1 >> 1) << 1;
    area->y1 = (y1 >> 1) << 1;

    // round the end of coordinate up to the nearest 2N+1 number
    area->x2 = ((x2 >> 1) << 1) + 1;
    area->y2 = ((y2 >> 1) << 1) + 1;
}

static void set_angle(void * obj, int32_t v) {
    lv_arc_set_value((lv_obj_t *)obj, v);
}

void lv_example_arc_2() {
    /*Create an Arc*/
    lv_obj_t * arc = lv_arc_create(lv_screen_active());
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(arc);

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, arc);
    lv_anim_set_exec_cb(&anim, set_angle);
    lv_anim_set_duration(&anim, 1000);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&anim, 500);
    lv_anim_set_values(&anim, 0, 100);
    lv_anim_start(&anim);
}

int main()
{
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

    lv_display_set_offset(nullptr, 6, 0);

    auto* act_scr = lv_display_get_default();
    lv_display_add_event_cb(act_scr, display_invalidate_cb, LV_EVENT_INVALIDATE_AREA, nullptr);

    lv_example_arc_2();

	display_blanking_off(display_dev);

	while (true) {
		uint32_t sleep_ms = lv_timer_handler();
		k_msleep(MIN(sleep_ms, INT32_MAX));

        // SystemInfo::print_heap_info();
        // SystemInfo::print_stack_info();
	}

	return 0;
}
