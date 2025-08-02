#include <memory>
#include <ctime>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
// #include <zephyr/drivers/display/display_co5300.h>

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

static void print(uint16_t color, size_t x = 100, size_t y = 100) {
    struct display_buffer_descriptor buf_desc;

    // Allocate correct size for RGB565
    size_t buf_size = 100 * 100 * 2;
    auto ext_buf = make_shared_ext<ExtVector>(buf_size);
    auto* ext_buf_p = (uint16_t*)ext_buf->data();

    // uint8_t* ext_buf = (uint8_t*)k_malloc(buf_size);
    // uint16_t* ext_buf_p = (uint16_t*)ext_buf;

    for (size_t idx = 0; idx < buf_size / 2; idx++)
        ext_buf_p[idx] = color;

    buf_desc.buf_size = buf_size;
    buf_desc.width = 100;
    buf_desc.height = 100;
    buf_desc.frame_incomplete = false;

    display_write(display_dev, x, y, &buf_desc, ext_buf_p);

    // SystemInfo::print_heap_info();
    // SystemInfo::print_stack_info();

    // k_free(ext_buf);
}

static void print_moving() {
    const size_t RECT_SIZE = 100;

    // Static variables to maintain position and direction between calls
    static int x = 10;
    static int y = 10;
    static int dx = 2;  // X velocity
    static int dy = 2;  // Y velocity

    const int center_x = DISPLAY_WIDTH / 2;
    const int center_y = DISPLAY_HEIGHT / 2;

    struct display_buffer_descriptor buf_desc;

    // Allocate correct size for RGB565
    size_t buf_size = RECT_SIZE * RECT_SIZE * 2;
    auto ext_buf = make_shared_ext<ExtVector>(buf_size);
    uint16_t* ext_buf_p = (uint16_t*)ext_buf->data();

    // Fill buffer with green color
    for (size_t idx = 0; idx < buf_size / 2; idx++)
        ext_buf_p[idx] = 0b0001111100000000;

    buf_desc.buf_size = buf_size;
    buf_desc.width = RECT_SIZE;
    buf_desc.height = RECT_SIZE;
    buf_desc.frame_incomplete = false;

    // Update position
    x += dx;
    y += dy;

    // Check if rectangle corners are within circular boundary
    // We need to check all four corners of the rectangle
    int corners_x[4] = {x, x + RECT_SIZE, x, x + RECT_SIZE};
    int corners_y[4] = {y, y, y + RECT_SIZE, y + RECT_SIZE};

    // Additional boundary checks for display edges (fallback)
    if (x <= 0 || x >= (int)(DISPLAY_WIDTH - RECT_SIZE)) {
        dx = -dx;
        x = (x <= 0) ? 1 : DISPLAY_WIDTH - RECT_SIZE - 1;
    }

    if (y <= 0 || y >= (int)(DISPLAY_HEIGHT - RECT_SIZE)) {
        dy = -dy;
        y = (y <= 0) ? 1 : DISPLAY_HEIGHT - RECT_SIZE - 1;
    }

    display_write(display_dev, x, y, &buf_desc, ext_buf_p);
}

// void blank_screen() {
//     display_buffer_descriptor buf_desc;

//     // Allocate correct size for RGB565
//     size_t buf_size = (DISPLAY_WIDTH / 4) * (DISPLAY_HEIGHT / 4) * 2;
//     auto ext_buf = make_shared_ext<ExtVector>(buf_size);

//     memset(ext_buf->data(), 0, ext_buf->size());

//     buf_desc.buf_size = buf_size;
//     buf_desc.width = DISPLAY_WIDTH / 4;
//     buf_desc.height = DISPLAY_HEIGHT / 4;

//     for(size_t x = 0; x < 4; x++)
//         for(size_t y = 0; y < 4; y++)
//             display_write(display_dev, x * (DISPLAY_WIDTH / 4), y * (DISPLAY_HEIGHT / 4), &buf_desc, ext_buf->data());
// }

void clear_screen() {
    display_buffer_descriptor buf_desc;

    // Allocate correct size for RGB565
    size_t buf_size = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2;
    auto ext_buf = make_shared_ext<ExtVector>(buf_size);

    memset(ext_buf->data(), 0, ext_buf->size());

    buf_desc.buf_size = buf_size;
    buf_desc.width = DISPLAY_WIDTH;
    buf_desc.height = DISPLAY_HEIGHT;

    display_write(display_dev, 0, 0, &buf_desc, ext_buf->data());
}

int main() {
    // LOG_INF("Starting sample");

    if (!device_is_ready(display_dev))
		LOG_ERR("Device %s not found. Aborting sample.", display_dev->name);

    display_set_pixel_format(display_dev, PIXEL_FORMAT_RGB_565);

    clear_screen();

    // display_set_brightness(display_dev, 0);

    // print(0b0001111100000000);
    // k_sleep(K_MSEC(1000));
    print(0b1110000011100000);

    // display_set_brightness(display_dev, 255);

    // auto fs_service = make_shared_ext<FsService>();

    // auto time_service = make_shared_ext<BootElapsedTimeService>();
    // time_service->Initialize();

    // auto system_config_service = make_shared_ext<ConfigurationService<SystemConfig>>("system_config", fs_service);
    // auto system_configuration_controller = make_shared_ext<SystemConfigurationController>(system_config_service);

    bool reverse = false;
    uint8_t brightness = 255;
    while (true) {
        // LOG_INF("Running sample");
        // SystemInfo::print_heap_info();
        // SystemInfo::print_stack_info();
        // k_msleep(SLEEP_TIME_MS);

        // print_moving();
        // k_msleep(50);

        display_set_brightness(display_dev, brightness);
        brightness = reverse ? brightness + 1 : brightness - 1;
        if(brightness == 255 || brightness == 0)
            reverse = !reverse;
        k_msleep(1);
    }

    return 0;
}
