#include <memory>
#include <ctime>
#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>

#include "utilities/memory/heap_allocator.h"
#include "utilities/dev_tools/system_info.h"
#include "utilities/guid/guid_generator.h"
// #include "utilities/time/boot_elapsed_time_service.h"

#include "subsys/modbus/modbus.h"

#include "domain/device_tree/device_tree_setup.h"
#include "domain/interface_domain/interface.h"
// #include "domain/fs_domain/services/fs_service.h"

// #include "configuration/system_config/system_config.h"
// #include "configuration/services/configuration_service.h"
// #include "controllers/system_configuration_controller.h"

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
using namespace eerie_leap::utilities::guid;
// using namespace eerie_leap::utilities::time;

// using namespace eerie_leap::controllers;

using namespace eerie_leap::subsys::modbus;

using namespace eerie_leap::domain::device_tree;
using namespace eerie_leap::domain::interface_domain;
// using namespace eerie_leap::domain::fs_domain::services;
// using namespace eerie_leap::configuration::services;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 2000;
const size_t DISPLAY_WIDTH = 466;
const size_t DISPLAY_HEIGHT = 466;

static const device* const display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// NOTE: Fixes incorrect invalidation of redrawn area
// https://forum.lvgl.io/t/lvgl-9-2-esp32-s3-display-axs15321b/19735/5
void display_invalidate_cb(lv_event_t* e) {
    auto* area = (lv_area_t*)lv_event_get_param(e);

    // Round down to even
    area->x1 &= ~1;
    area->y1 &= ~1;

    // Round up to odd
    area->x2 |= 1;
    area->y2 |= 1;
}

static lv_obj_t* ui_label = nullptr;

void update_label(void* obj, int32_t v) {
    lv_arc_set_value((lv_obj_t*)obj, v);
    lv_label_set_text(ui_label, std::to_string(v).c_str());
}

lv_obj_t* create_ui_arc() {
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

    ui_label = lv_label_create(lv_screen_active());
    lv_obj_set_width(ui_label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_label, "0");
    lv_obj_set_style_text_font(ui_label, &lv_font_unscii_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_transform_scale(ui_label, 1500, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(ui_label, -65);
    lv_obj_set_y(ui_label, -55);


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

int main()
{
    DeviceTreeSetup::Initialize();
    auto& device_tree_setup = DeviceTreeSetup::Get();

    auto guid_generator = make_shared_ext<GuidGenerator>();

    auto modbus = make_shared_ext<Modbus>(device_tree_setup.GetModbusIface().value());
    auto interface = make_shared_ext<Interface>(modbus, guid_generator);
    interface->Initialize();

	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return 0;
	}

    lv_display_set_offset(nullptr, 6, 0);

    auto* act_scr = lv_display_get_default();
    lv_display_add_event_cb(act_scr, display_invalidate_cb, LV_EVENT_INVALIDATE_AREA, nullptr);

    auto* ui_arc = create_ui_arc();
    interface->RegisterReadingHandler(2348664336, [ui_arc](SensorReadingDto& reading) {
        update_label(ui_arc, static_cast<int>(reading.value));

        return 0;
    });

	display_blanking_off(display_dev);

	while (true) {
		uint32_t sleep_ms = lv_timer_handler();
		k_msleep(MIN(sleep_ms, INT32_MAX));

        // SystemInfo::print_heap_info();
        // SystemInfo::print_stack_info();
	}

	return 0;
}
