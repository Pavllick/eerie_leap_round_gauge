#pragma once

#include <cstddef>

#include "subsys/event_bus/event_channel.h"

#include "ui_signal_type.h"
#include "ui_signal_payload_type.h"

namespace eerie_leap::domain::ui_domain::event_bus {

using eerie_leap::subsys::event_bus::EventChannel;

// Fan-in of every domain event the UI is allowed to bind to. Widgets subscribe by the
// UiSignalType stored in their configuration; only UiSignalBridge publishes here.
class UiSignalChannel : public EventChannel<UiSignalType, UiSignalPayloadType> {
private:
    static constexpr size_t k_max_queued_events = 16;

    UiSignalChannel() : EventChannel("ui_signals", k_max_queued_events) { }

public:
    static UiSignalChannel& GetInstance() {
        static UiSignalChannel channel;

        return channel;
    }
};

} // namespace eerie_leap::domain::ui_domain::event_bus
