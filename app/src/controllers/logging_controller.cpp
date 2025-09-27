#include "domain/interface_domain/types/com_user_status.h"

#include "logging_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;

LoggingController::LoggingController(
    std::shared_ptr<GpioButtons> gpio_buttons,
    std::shared_ptr<Interface> interface,
    std::shared_ptr<GagueController> gague_controller)
    : gpio_buttons_(std::move(gpio_buttons)),
    interface_(std::move(interface)),
    gague_controller_(std::move(gague_controller)),
    is_logging_in_progress_(false) {}

int LoggingController::Initialize() {
    gpio_buttons_->RegisterCallback(0, [&]() {
        interface_->SetStatus(!is_logging_in_progress_
            ? ComUserStatus::START_LOGGING
            : ComUserStatus::STOP_LOGGING);

        return 0;
    });

    interface_->RegisterStatusUpdateHandler(ComUserStatus::START_LOGGING,
        [&is_logging_in_progress = is_logging_in_progress_,
            &gague_controller = gague_controller_](ComUserStatus status, bool success) {

        if(success) {
            is_logging_in_progress = true;
            gague_controller->UpdateWidgetProperty(
                WidgetPropertyType::IS_VISIBLE,
                true,
                WidgetTag::IconLoggingIndicator,
                true);
        }

        return 0;
    });

    interface_->RegisterStatusUpdateHandler(ComUserStatus::STOP_LOGGING,
        [&is_logging_in_progress = is_logging_in_progress_,
            &gague_controller = gague_controller_](ComUserStatus status, bool success) {

        if(success) {
            is_logging_in_progress = false;
            gague_controller->UpdateWidgetProperty(
                WidgetPropertyType::IS_VISIBLE,
                false,
                WidgetTag::IconLoggingIndicator,
                true);
        }

        return 0;
    });

    return 0;
}

} // namespace eerie_leap::controllers
