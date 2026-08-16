#include "domain/canbus_com_domain/commands/canbus_com_logging_command.h"

#include "logging_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::canbus_com_domain::commands;

LoggingController::LoggingController(
    std::shared_ptr<IGpio> gpio,
    std::shared_ptr<WorkQueueThread> input_work_queue_thread,
    std::shared_ptr<CanbusComService> canbus_com_service)
        : gpio_(std::move(gpio)),
        input_work_queue_thread_(std::move(input_work_queue_thread)),
        canbus_com_service_(std::move(canbus_com_service)),
        is_logging_in_progress_(false) {}

LoggingController::~LoggingController() {
    if(button_handler_id_ > 0)
        gpio_->RemoveChannelChangedHandler(LOGGING_BUTTON_CHANNEL, button_handler_id_);
}

int LoggingController::Initialize() {
    int handler_id = gpio_->RegisterChannelChangedHandler(
        LOGGING_BUTTON_CHANNEL,
        GpioEdge::ACTIVE,
        [this](int channel, bool state) {
            ARG_UNUSED(channel);
            ARG_UNUSED(state);

            input_work_queue_thread_->Run([this]() {
                CanbusComLoggingCommand command(!is_logging_in_progress_);
                canbus_com_service_->SendCommand(
                    command,
                    [this](bool success) { LoggingStateUpdatedAck(success); });
            });
        });

    if(handler_id <= 0)
        return handler_id;

    button_handler_id_ = handler_id;

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
