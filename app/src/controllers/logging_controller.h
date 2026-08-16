#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "subsys/gpio/i_gpio.h"
#include "subsys/threading/work_queue_thread.h"

#include "domain/canbus_com_domain/services/canbus_com_service.h"
#include "domain/ui_domain/event_bus/ui_event_bus.h"

namespace eerie_leap::controllers {

using eerie_leap::subsys::gpio::IGpio;
using eerie_leap::subsys::gpio::GpioEdge;
using eerie_leap::subsys::threading::WorkQueueThread;

using eerie_leap::domain::canbus_com_domain::services::CanbusComService;

using eerie_leap::domain::ui_domain::event_bus::UiEventBus;
using eerie_leap::domain::ui_domain::event_bus::UiEvent;
using eerie_leap::domain::ui_domain::event_bus::UiEventType;
using eerie_leap::domain::ui_domain::event_bus::UiEventPayload;
using eerie_leap::domain::ui_domain::event_bus::UiPayloadType;

class LoggingController {
private:
    static constexpr int LOGGING_BUTTON_CHANNEL = 0;

    std::shared_ptr<IGpio> gpio_;
    std::shared_ptr<WorkQueueThread> input_work_queue_thread_;
    std::shared_ptr<CanbusComService> canbus_com_service_;

    int button_handler_id_ = 0;
    bool is_logging_in_progress_ = false;

    void LoggingStateUpdatedAck(bool success);
    void PublishLoggingState(bool is_logging);

public:
    LoggingController(
        std::shared_ptr<IGpio> gpio,
        std::shared_ptr<WorkQueueThread> input_work_queue_thread,
        std::shared_ptr<CanbusComService> canbus_com_service);
    ~LoggingController();

    int Initialize();
};

} // namespace eerie_leap::controllers
