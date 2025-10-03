#include <zephyr/logging/log.h>

#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "status_processor.h"

namespace eerie_leap::domain::interface_domain::processors {

using namespace eerie_leap::domain::ui_domain::event_bus;

LOG_MODULE_REGISTER(status_processor_logger);

StatusProcessor::StatusProcessor() { }

void StatusProcessor::SubmitToEventBus(const UserStatus& status) {
    UiEventPayload payload;
    payload[UiPayloadType::IsStatusOk] = status.is_ok;
    payload[UiPayloadType::ComUserStatus] = static_cast<uint32_t>(status.status);

    UiEvent event {
        .type = UiEventType::StatusUpdated,
        .payload = payload
    };

    UiEventBus::GetInstance().PublishAsync(event);
}

int StatusProcessor::Process(UserStatus status) {
    SubmitToEventBus(status);

    return 0;
}

} // namespace eerie_leap::domain::interface_domain::services
