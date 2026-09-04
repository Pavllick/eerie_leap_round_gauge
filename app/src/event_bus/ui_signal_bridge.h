#pragma once

#include <vector>

#include "subsys/event_bus/event_channel.h"

#include "domain/ui_domain/event_bus/ui_signal_channel.h"

namespace eerie_leap::event_bus {

using eerie_leap::domain::ui_domain::event_bus::UiSignalType;
using eerie_leap::domain::ui_domain::event_bus::UiSignalChannel;
using eerie_leap::domain::ui_domain::event_bus::UiSignalPayloadType;
using eerie_leap::subsys::event_bus::AnySubscription;
using eerie_leap::subsys::event_bus::CreateScopedSubscription;

// The one place that knows which channel and payload key each UI signal comes from.
// Adding a signal touches this file and the UiSignalType enum, never a widget.
class UiSignalBridge {
private:
    std::vector<AnySubscription> links_;

    // Republishes synchronously, so subscribers run on the source channel's bus thread.
    // Swap a link to PublishAsync to hand that signal over to the UI bus thread instead.
    template<typename TChannel>
    void Link(
        TChannel& channel,
        typename TChannel::EventTypeEnum type,
        typename TChannel::PayloadTypeEnum key,
        UiSignalType signal) {

        links_.push_back(CreateScopedSubscription(
            channel,
            type,
            [key, signal](const typename TChannel::EventMessage& event) {
                auto it = event.payload.find(key);
                if(it == event.payload.end())
                    return;

                UiSignalChannel::GetInstance().Publish({
                    .type = signal,
                    .payload = { { UiSignalPayloadType::Value, it->second } }
                });
            }));
    }

public:
    void Initialize();
};

} // namespace eerie_leap::event_bus
