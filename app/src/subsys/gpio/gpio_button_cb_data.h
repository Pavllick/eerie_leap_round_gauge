#pragma once

#include <vector>

#include <zephyr/drivers/gpio.h>

#include "gpio_button_handler.h"

namespace eerie_leap::subsys::gpio {

struct GpioButtonCbData {
    gpio_callback callback;
    std::vector<GptioButtonHandler> handlers;
};

}  // namespace eerie_leap::subsys::gpio
