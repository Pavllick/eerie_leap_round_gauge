#include <functional>

#include <zephyr/logging/log.h>

#include "gpio_buttons.h"

LOG_MODULE_REGISTER(gpio_buttons_logger);

namespace eerie_leap::subsys::gpio {

int GpioButtons::Initialize() {
    LOG_INF("Gpio initialization started.");

    for(auto& gpio_spec : gpio_specs_) {
        int ret = InitializeButton(gpio_spec);
        if (ret != 0)
            return ret;
    }

    LOG_INF("Gpio initialized successfully.");

    return 0;
}

void GpioButtons::ButtonPressedCallback(const struct device* dev, gpio_callback* callback, uint32_t pins) {
    GpioButtonCbData* cb_data = CONTAINER_OF(callback, GpioButtonCbData, callback);

    int64_t now = k_uptime_get();

    if(now - cb_data->last_press_time < cb_data->DEBOUNCE_MS)
        return;

    cb_data->last_press_time = now;

    for(auto& handler : cb_data->handlers) {
        handler();
    }
}

bool GpioButtons::RegisterCallback(int index, GptioButtonHandler handler) {
    if(index < 0 || index >= gpio_specs_.size()) {
        LOG_ERR("Invalid Button index: %d", index);
        return false;
    }

    if(cb_data_containers_.find(index) == cb_data_containers_.end()) {
        cb_data_containers_[index] = std::make_shared<GpioButtonCbData>();

        auto& button = gpio_specs_.at(index);
        gpio_init_callback(&cb_data_containers_[index]->callback,
            ButtonPressedCallback,
            BIT(button.pin));
        gpio_add_callback(button.port, &cb_data_containers_[index]->callback);
    }

    cb_data_containers_[index]->handlers.push_back(handler);

    return true;
}

int GpioButtons::Count() {
    return gpio_specs_.size();
}

int GpioButtons::InitializeButton(gpio_dt_spec& button) {
    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("Button GPIO device not ready.");
        return -1;
    } else {
        LOG_INF("Button initialized: port %s, pin %d",
            button.port->name, button.pin);

        int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
        if (ret != 0) {
            LOG_ERR("Error %d: failed to configure %s pin %d\n",
                ret, button.port->name, button.pin);

            return ret;
        }

        ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
        if (ret != 0) {
            LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
                ret, button.port->name, button.pin);

            return ret;
        }
    }

    return 0;
}

}  // namespace eerie_leap::subsys::gpio
