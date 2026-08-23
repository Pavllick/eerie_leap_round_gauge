#pragma once

#include <cstdint>
#include <variant>

#include "domain/ui_domain/event_bus/ui_event_bus.h"

namespace eerie_leap::views::widgets::event_bus_filters {

using eerie_leap::domain::ui_domain::event_bus::UiEvent;
using eerie_leap::domain::ui_domain::event_bus::UiPayloadType;

struct SettingFilter {
    uint32_t setting_id;

    bool operator()(const UiEvent& event) const {
        if(auto it = event.payload.find(UiPayloadType::SettingId); it != event.payload.end()) {
            if(auto id = std::get_if<uint32_t>(&it->second))
                return *id == setting_id;
        }

        return false;
    }
};

} // namespace eerie_leap::views::widgets::event_bus_filters
