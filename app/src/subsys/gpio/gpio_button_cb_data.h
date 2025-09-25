#pragma once

#include <vector>

#include <zephyr/drivers/gpio.h>

#include "gpio_button_handler.h"

namespace eerie_leap::subsys::gpio {

struct GpioButtonCbData {
    gpio_callback callback;
    std::vector<GptioButtonHandler> handlers;
    int64_t last_press_time = 0;
    static constexpr int DEBOUNCE_MS = 50;
};

}  // namespace eerie_leap::subsys::gpio
