#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "subsys/device_tree/dt_gpio.h"
#include "subsys/gpio/gpio_buttons.h"
#include "subsys/threading/work_queue_thread.h"

#include "domain/canbus_com_domain/services/canbus_com_service.h"
#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "controllers/ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::subsys::device_tree;
using namespace eerie_leap::subsys::gpio;
using namespace eerie_leap::subsys::threading;

using namespace eerie_leap::domain::canbus_com_domain::services;
using namespace eerie_leap::domain::ui_domain::event_bus;

class LoggingController {
private:
    std::shared_ptr<GpioButtons> gpio_buttons_;
    std::shared_ptr<WorkQueueThread> input_work_queue_thread_;
    std::shared_ptr<CanbusComService> canbus_com_service_;

    bool is_logging_in_progress_ = false;

    void LoggingStateUpdatedAck(bool success);
    void PublishLoggingState(bool is_logging);

public:
    LoggingController(
        std::shared_ptr<GpioButtons> gpio_buttons,
        std::shared_ptr<WorkQueueThread> input_work_queue_thread,
        std::shared_ptr<CanbusComService> canbus_com_service);

    int Initialize();
};

} // namespace eerie_leap::controllers
