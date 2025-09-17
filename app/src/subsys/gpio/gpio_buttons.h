#pragma once

#include <memory>
#include <vector>

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "gpio_button_cb_data.h"

namespace eerie_leap::subsys::gpio {

class GpioButtons {
protected:
    std::vector<gpio_dt_spec> gpio_specs_;
    std::unordered_map<int, std::shared_ptr<GpioButtonCbData>> cb_data_containers_;

    static int InitializeButton(gpio_dt_spec& button);

public:
    GpioButtons(std::vector<gpio_dt_spec> gpio_specs) : gpio_specs_(gpio_specs) {}

    int Initialize();
    bool RegisterCallback(int index, GptioButtonHandler handler);
    int Count();
};

}  // namespace eerie_leap::subsys::gpio
