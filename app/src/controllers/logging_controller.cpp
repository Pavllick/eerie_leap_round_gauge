#include "controllers/event_bus_filters/user_status_filter.h"

#include "logging_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;
using namespace eerie_leap::controllers::event_bus_filters;

LoggingController::LoggingController(std::shared_ptr<GpioButtons> gpio_buttons, std::shared_ptr<Interface> interface)
    : gpio_buttons_(std::move(gpio_buttons)), interface_(std::move(interface)), is_logging_in_progress_(false) { }

int LoggingController::Initialize() {
    gpio_buttons_->RegisterCallback(0, [&]() {
        interface_->SetStatus(!is_logging_in_progress_
            ? ComUserStatus::START_LOGGING
            : ComUserStatus::STOP_LOGGING);

        return 0;
    });

    auto start_logging_subscription = UiEventBus::GetInstance().Subscribe(
        UiEventType::STATUS_UPDATED,
        UserStatusFilter { ComUserStatus::START_LOGGING },
        [this](const UiEvent& event) {
            if (auto it = event.payload.find(UiPayloadType::IS_STATUS_OK); it != event.payload.end()) {
                if (auto* value = std::get_if<bool>(&it->second)) {
                    if(*value) {
                        is_logging_in_progress_ = true;
                        PublishLoggingState(is_logging_in_progress_);
                    }
                }
            }
        }
    );

    if(start_logging_subscription)
        subscriptions_.push_back(std::move(*start_logging_subscription));

    auto stop_logging_subscription = UiEventBus::GetInstance().Subscribe(
        UiEventType::STATUS_UPDATED,
        UserStatusFilter { ComUserStatus::STOP_LOGGING },
        [this](const UiEvent& event) {
            if (auto it = event.payload.find(UiPayloadType::IS_STATUS_OK); it != event.payload.end()) {
                if (auto* value = std::get_if<bool>(&it->second)) {
                    if(*value) {
                        is_logging_in_progress_ = false;
                        PublishLoggingState(is_logging_in_progress_);
                    }
                }
            }
        }
    );

    if(stop_logging_subscription)
        subscriptions_.push_back(std::move(*stop_logging_subscription));

    return 0;
}

void LoggingController::PublishLoggingState(bool is_logging) {
    UiEventPayload payload;
    payload[UiPayloadType::VALUE] = is_logging;

    UiEvent event {
        .type = UiEventType::LOGGING_STATUS_UPDATED,
        .payload = payload
    };

    UiEventBus::GetInstance().PublishAsync(event);
}

} // namespace eerie_leap::controllers
