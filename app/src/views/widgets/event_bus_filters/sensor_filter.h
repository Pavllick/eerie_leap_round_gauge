#pragma once

#include <cstdint>
#include <variant>

#include "domain/sensor_domain/event_bus/sensor_events_channel.h"

namespace eerie_leap::views::widgets::event_bus_filters {

using eerie_leap::domain::sensor_domain::event_bus::SensorEventsChannel;
using eerie_leap::domain::sensor_domain::event_bus::SensorPayloadType;

struct SensorFilter {
    uint32_t sensor_id;

    bool operator()(const SensorEventsChannel::EventMessage& event) const {
        if(auto it = event.payload.find(SensorPayloadType::SensorId); it != event.payload.end()) {
            if(auto id = std::get_if<uint32_t>(&it->second))
                return *id == sensor_id;
        }

        return false;
    }
};

} // namespace eerie_leap::views::widgets::event_bus_filters
