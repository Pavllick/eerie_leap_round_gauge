#include <memory>
#include <ctime>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
// #include <zephyr/drivers/display/display_co5300.h>

// #include "utilities/memory/heap_allocator.h"
#include "utilities/dev_tools/system_info.h"
// #include "utilities/time/boot_elapsed_time_service.h"
// #include "domain/fs_domain/services/fs_service.h"

// #include "configuration/system_config/system_config.h"
// #include "configuration/services/configuration_service.h"
// #include "controllers/system_configuration_controller.h"

// using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::utilities::dev_tools;
// using namespace eerie_leap::utilities::time;

// using namespace eerie_leap::controllers;

// using namespace eerie_leap::domain::fs_domain::services;
// using namespace eerie_leap::configuration::services;

LOG_MODULE_REGISTER(main_logger);

constexpr uint32_t SLEEP_TIME_MS = 2000;

static const device* const display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static void print() {
    const size_t DISPLAY_WIDTH = 466;
    const size_t DISPLAY_HEIGHT = 466;
    const size_t RECT_WIDTH = 50;
    const size_t RECT_HEIGHT = 200;

    // Center coordinates
    size_t x = 200;//(DISPLAY_WIDTH - RECT_WIDTH) / 2;
    size_t y = 200;//(DISPLAY_HEIGHT - RECT_HEIGHT) / 2;

    struct display_buffer_descriptor buf_desc;

    // Allocate correct size for RGB565
    // size_t buf_size = 4 * 4 * 2;
    // uint8_t* buf = (uint8_t*)k_malloc(buf_size);
    size_t buf_size = 4 * 4 * 2;
    uint16_t* buf = (uint16_t*)k_malloc(buf_size);

    if (!buf) {
        printk("Failed to allocate buffer\n");
        return;
    }

    for (size_t idx = 0; idx < buf_size; idx++)
        buf[idx] = 0b0001111100000000;

    buf_desc.buf_size = buf_size;
    buf_desc.width = 4;
    buf_desc.height = 4;
    buf_desc.frame_incomplete = false;

    display_write(display_dev, x, y, &buf_desc, buf);

    k_free(buf);
}


int main() {
    // LOG_INF("Starting sample");

    if (!device_is_ready(display_dev))
		LOG_ERR("Device %s not found. Aborting sample.", display_dev->name);

    display_set_pixel_format(display_dev, PIXEL_FORMAT_RGB_565);

    print();

    // auto fs_service = make_shared_ext<FsService>();

    // auto time_service = make_shared_ext<BootElapsedTimeService>();
    // time_service->Initialize();

    // auto system_config_service = make_shared_ext<ConfigurationService<SystemConfig>>("system_config", fs_service);
    // auto system_configuration_controller = make_shared_ext<SystemConfigurationController>(system_config_service);

    while (true) {
        LOG_INF("Running sample");
        // SystemInfo::print_heap_info();
        // SystemInfo::print_stack_info();
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
