#pragma once

#include "subsys/event_bus/event_bus.h"

#include "domain/ui_domain/lvgl_lock.h"

#include "ui_event_type.h"
#include "ui_payload_type.h"

namespace eerie_leap::domain::ui_domain::event_bus {

namespace event_bus = eerie_leap::subsys::event_bus;

using UiEventPayload = event_bus::EventPayload<UiPayloadType>;
using UiEvent = event_bus::Event<UiEventType, UiPayloadType>;
using UiSubscriptionHandle = event_bus::SubscriptionHandle<UiEventType>;

class UiEventBus : public event_bus::EventBus<UiEventType, UiPayloadType> {
private:
    static constexpr int event_bus_stack_size_ = 4096;

    UiEventBus() : event_bus::EventBus<UiEventType, UiPayloadType>(
        "ui_event_bus",
        event_bus_stack_size_,
        []() { LvglLock::GetInstance().Lock(); },
        []() { LvglLock::GetInstance().Unlock(); }) { }

public:
    static UiEventBus& GetInstance() {
        static UiEventBus bus;
        return bus;
    }
};

} // namespace eerie_leap::domain::ui_domain::event_bus
