#pragma once

#include "subsys/event_bus/event_bus.h"

#include "ui_event_type.h"
#include "ui_payload_type.h"

namespace eerie_leap::domain::ui_domain::event_bus {

using namespace eerie_leap::subsys::event_bus;

using UiEventPayload = EventPayload<UiPayloadType>;
using UiEvent = Event<UiEventType, UiPayloadType>;
using UiSubscriptionHandle = SubscriptionHandle<UiEventType>;

class UiEventBus : public EventBus<UiEventType, UiPayloadType> {
private:
    UiEventBus() : EventBus<UiEventType, UiPayloadType>("ui_event_bus") { }

public:
    static UiEventBus& GetInstance() {
        static UiEventBus bus;

        return bus;
    }
};

} // namespace eerie_leap::domain::ui_domain::event_bus
