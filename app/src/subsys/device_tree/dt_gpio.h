#pragma once

#include <vector>
#include <optional>

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include "zephyr/drivers/gpio.h"

#if DT_HAS_ALIAS(gpio0buttons)
#define GPIO0BUTTONS_NODE DT_ALIAS(gpio0buttons)
#define GPIO_SPEC(node_id) GPIO_DT_SPEC_GET(node_id, gpios)
#endif

namespace eerie_leap::subsys::device_tree {

class DtGpio {
private:
    static std::optional<std::vector<gpio_dt_spec>> gpio_input_specs_;

    DtGpio() = default;

    static void InitializeButton(gpio_dt_spec& button);

public:
    static void Initialize();

    static std::optional<std::vector<gpio_dt_spec>>& GetButtons() { return gpio_input_specs_; }
};

} // namespace eerie_leap::subsys::device_tree
