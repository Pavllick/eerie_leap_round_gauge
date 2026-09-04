#pragma once

#include <array>
#include <cstddef>

#include "subsys/event_bus/i_event_channel.h"

#include "event_channel_id.h"

namespace eerie_leap::event_bus {

using eerie_leap::subsys::event_bus::IEventChannel;

// Resolves the channel id persisted in a widget binding onto the channel itself. Observes
// rather than owns: every channel is a singleton that outlives the registry.
class EventChannelRegistry {
private:
    static constexpr size_t k_max_channels = 8;

    std::array<IEventChannel*, k_max_channels> channels_{};

    EventChannelRegistry() = default;

public:
    static EventChannelRegistry& GetInstance();

    EventChannelRegistry(const EventChannelRegistry&) = delete;
    EventChannelRegistry& operator=(const EventChannelRegistry&) = delete;

    int Register(EventChannelId id, IEventChannel& channel);

    // Null for an id nothing was registered under, so a stale binding is inert rather than fatal.
    IEventChannel* Find(EventChannelId id) const;
};

} // namespace eerie_leap::event_bus
