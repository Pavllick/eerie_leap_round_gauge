#pragma once

#include <functional>

#include <zephyr/drivers/gpio.h>

namespace eerie_leap::subsys::gpio {

using GptioButtonHandler = std::function<int()>;

}  // namespace eerie_leap::subsys::gpio
