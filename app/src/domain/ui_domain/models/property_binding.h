#pragma once

#include <cstdint>
#include <variant>

#include "utilities/type/config_value.h"

#include "event_bus/event_channel_id.h"

#include "property_binding_direction.h"
#include "widget_property.h"

namespace eerie_leap::domain::ui_domain::models {

using eerie_leap::event_bus::EventChannelId;
using eerie_leap::utilities::type::ConfigValue;

struct PropertyBinding {
    WidgetPropertyType target = WidgetPropertyType::NONE;

    EventChannelId channel = EventChannelId::None;

    // Channel-local enum values, erased to match IEventChannel::SubscribeErased.
    uint32_t event_type = 0;
    uint32_t payload_key = 0;

    PropertyBindingDirection direction = PropertyBindingDirection::In;

    // The event published when the user moves the widget. Unused by an In binding.
    uint32_t outbound_event_type = 0;

    // Narrows the subscription to one sensor or one setting. The key cannot signal absence on its
    // own, because 0 is a live payload key on every channel, so an unset value carries that.
    uint32_t selector_key = 0;
    ConfigValue selector_value;

    bool HasSelector() const {
        return !std::holds_alternative<std::monostate>(selector_value);
    }
};

} // namespace eerie_leap::domain::ui_domain::models
