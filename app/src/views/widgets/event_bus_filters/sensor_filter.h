#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "domain/ui_domain/event_bus/ui_event_bus.h"

namespace eerie_leap::views::widgets::event_bus_filters {

using namespace eerie_leap::domain::ui_domain::event_bus;

struct SensorFilter {
    uint32_t sensor_id;

    bool operator()(const UiEvent& event) const {
        if (auto it = event.payload.find(UiPayloadType::SensorId); it != event.payload.end()) {
            if (auto id = std::get_if<uint32_t>(&it->second)) {
                return *id == sensor_id;
            }
        }

        return false;
    }
};

} // namespace eerie_leap::views::widgets::event_bus_filters
