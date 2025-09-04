#include <zephyr/logging/log.h>

#include "dt_gpio.h"

namespace eerie_leap::subsys::device_tree {

LOG_MODULE_REGISTER(dt_gpio_logger);

std::optional<std::vector<gpio_dt_spec>> DtGpio::gpio_input_specs_ = std::nullopt;

void DtGpio::Initialize() {
#if DT_HAS_ALIAS(gpio0buttons)
    gpio_input_specs_ = { DT_FOREACH_CHILD_SEP(GPIO0BUTTONS_NODE, GPIO_SPEC, (,)) };
    LOG_INF("GPIO initialized.");

    for(auto& gpio_spec : gpio_input_specs_.value())
        InitializeButton(gpio_spec);
#endif
}

void DtGpio::InitializeButton(gpio_dt_spec& button) {
    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("Button GPIO device not ready.");
    } else {
        LOG_INF("Button initialized: port %s, pin %d",
            button.port->name, button.pin);

        int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
        if (ret != 0) {
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
            ret, button.port->name, button.pin);
        }

        ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
        if (ret != 0) {
            LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
                ret, button.port->name, button.pin);
        }
    }
}

} // namespace eerie_leap::subsys::device_tree
