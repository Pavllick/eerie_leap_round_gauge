#include <zephyr/logging/log.h>

#include "dt_gpio.h"

namespace eerie_leap::subsys::device_tree {

LOG_MODULE_REGISTER(dt_gpio_logger);

std::optional<std::vector<gpio_dt_spec>> DtGpio::gpio_input_specs_ = std::nullopt;

void DtGpio::Initialize() {
#if DT_HAS_ALIAS(gpio0buttons)
    gpio_input_specs_ = { DT_FOREACH_CHILD_SEP(GPIO0BUTTONS_NODE, GPIO_SPEC, (,)) };
    LOG_INF("GPIO initialized.");
#endif
}

} // namespace eerie_leap::subsys::device_tree
