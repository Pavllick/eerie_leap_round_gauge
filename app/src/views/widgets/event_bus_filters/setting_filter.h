#pragma once

#include <cstdint>
#include <variant>

#include "domain/settings_domain/event_bus/settings_events_channel.h"

namespace eerie_leap::views::widgets::event_bus_filters {

using eerie_leap::domain::settings_domain::event_bus::SettingsEventsChannel;
using eerie_leap::domain::settings_domain::event_bus::SettingsPayloadType;

struct SettingFilter {
    uint32_t setting_id;

    bool operator()(const SettingsEventsChannel::EventMessage& event) const {
        if(auto it = event.payload.find(SettingsPayloadType::SettingId); it != event.payload.end()) {
            if(auto id = std::get_if<uint32_t>(&it->second))
                return *id == setting_id;
        }

        return false;
    }
};

} // namespace eerie_leap::views::widgets::event_bus_filters
