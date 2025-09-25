#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "subsys/device_tree/dt_gpio.h"
#include "subsys/gpio/gpio_buttons.h"
#include "domain/interface_domain/interface.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::gpio;
using namespace eerie_leap::domain::interface_domain;

class GpioController {
private:
    std::shared_ptr<GpioButtons> gpio_buttons_;
    std::shared_ptr<Interface> interface_;

    bool is_logging_in_progress_;

public:
    GpioController(std::shared_ptr<GpioButtons> gpio_buttons, std::shared_ptr<Interface> interface);

    int Initialize();
};

} // namespace eerie_leap::controllers
