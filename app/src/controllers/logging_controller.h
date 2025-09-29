#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "subsys/device_tree/dt_gpio.h"
#include "subsys/gpio/gpio_buttons.h"
#include "domain/interface_domain/interface.h"
#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "controllers/ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::gpio;
using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::ui_domain::event_bus;

class LoggingController {
private:
    std::shared_ptr<GpioButtons> gpio_buttons_;
    std::shared_ptr<Interface> interface_;
    std::vector<UiSubscriptionHandle> subscriptions_;

    bool is_logging_in_progress_ = false;

    void PublishLoggingState(bool is_logging);

public:
    LoggingController(std::shared_ptr<GpioButtons> gpio_buttons, std::shared_ptr<Interface> interface);

    int Initialize();
};

} // namespace eerie_leap::controllers
