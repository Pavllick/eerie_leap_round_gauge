#include "domain/canbus_com_domain/commands/canbus_com_logging_command.h"

#include "logging_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::canbus_com_domain::commands;

LoggingController::LoggingController(
    std::shared_ptr<GpioButtons> gpio_buttons,
    std::shared_ptr<WorkQueueThread> input_work_queue_thread,
    std::shared_ptr<CanbusComService> canbus_com_service)
        : gpio_buttons_(std::move(gpio_buttons)),
        input_work_queue_thread_(std::move(input_work_queue_thread)),
        canbus_com_service_(std::move(canbus_com_service)),
        is_logging_in_progress_(false) {}

int LoggingController::Initialize() {
    gpio_buttons_->RegisterCallback(0, [this]() {
        input_work_queue_thread_->Run([this]() {
            CanbusComLoggingCommand command(!is_logging_in_progress_);
            canbus_com_service_->SendCommand(
                command,
                [this](bool success) { LoggingStateUpdatedAck(success); });
        });

        return 0;
    });

    return 0;
}

void LoggingController::LoggingStateUpdatedAck(bool success) {
    if(!success)
        return;

    is_logging_in_progress_ = !is_logging_in_progress_;
    PublishLoggingState(is_logging_in_progress_);
}

void LoggingController::PublishLoggingState(bool is_logging) {
    UiEventPayload payload;
    payload[UiPayloadType::Value] = is_logging;

    UiEvent event {
        .type = UiEventType::LoggingStatusUpdated,
        .payload = payload
    };

    UiEventBus::GetInstance().PublishAsync(event);
}

} // namespace eerie_leap::controllers
