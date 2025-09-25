#include "domain/interface_domain/types/com_user_status.h"

#include "gpio_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;

GpioController::GpioController(std::shared_ptr<GpioButtons> gpio_buttons, std::shared_ptr<Interface> interface)
    : gpio_buttons_(std::move(gpio_buttons)), interface_(std::move(interface)), is_logging_in_progress_(false) {}

int GpioController::Initialize() {
    gpio_buttons_->RegisterCallback(0, [&]() {
        interface_->SetStatus(!is_logging_in_progress_
            ? ComUserStatus::START_LOGGING
            : ComUserStatus::STOP_LOGGING);

        is_logging_in_progress_ = !is_logging_in_progress_;

        return 0;
    });

    return 0;
}

} // namespace eerie_leap::controllers
