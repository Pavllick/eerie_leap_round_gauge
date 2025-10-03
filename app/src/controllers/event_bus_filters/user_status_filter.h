#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "domain/interface_domain/types/user_status.h"

namespace eerie_leap::controllers::event_bus_filters {

using namespace eerie_leap::domain::ui_domain::event_bus;
using namespace eerie_leap::domain::interface_domain::types;

struct UserStatusFilter {
    ComUserStatus user_status;

    bool operator()(const UiEvent& event) const {
        if (auto it = event.payload.find(UiPayloadType::ComUserStatus); it != event.payload.end()) {
            if (auto status = std::get_if<uint32_t>(&it->second)) {
                return static_cast<ComUserStatus>(*status) == user_status;
            }
        }

        return false;
    }
};

} // namespace eerie_leap::controllers::event_bus_filters
