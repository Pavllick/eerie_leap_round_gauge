#include <cerrno>

#include <zephyr/logging/log.h>

#include "event_channel_registry.h"

namespace eerie_leap::event_bus {

LOG_MODULE_REGISTER(event_channel_registry_logger);

EventChannelRegistry& EventChannelRegistry::GetInstance() {
    static EventChannelRegistry registry;

    return registry;
}

int EventChannelRegistry::Register(EventChannelId id, IEventChannel& channel) {
    auto index = static_cast<size_t>(id);

    if(id == EventChannelId::None || index >= k_max_channels) {
        LOG_ERR("Channel id %u cannot be registered.", static_cast<unsigned>(index));
        return -EINVAL;
    }

    // Registration happens during composition, where a return value has nowhere to go.
    if(channels_[index] != nullptr && channels_[index] != &channel) {
        LOG_ERR("Channel id %u is already held by '%s'.",
            static_cast<unsigned>(index), channels_[index]->GetName());

        return -EEXIST;
    }

    channels_[index] = &channel;

    return 0;
}

IEventChannel* EventChannelRegistry::Find(EventChannelId id) const {
    auto index = static_cast<size_t>(id);

    if(id == EventChannelId::None || index >= k_max_channels)
        return nullptr;

    return channels_[index];
}

} // namespace eerie_leap::event_bus
