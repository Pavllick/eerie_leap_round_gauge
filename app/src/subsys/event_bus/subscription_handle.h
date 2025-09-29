#pragma once

#include <cstddef>

#include "event.h"

namespace eerie_leap::subsys::event_bus {

template<EnumClassUint32 EventTypeEnum>
class SubscriptionHandle {
private:
    size_t id_;
    EventTypeEnum event_type_;
    bool valid_;

public:
    SubscriptionHandle(size_t id, EventTypeEnum type)
        : id_(id), event_type_(type), valid_(true) {}

    SubscriptionHandle(const SubscriptionHandle&) = delete;
    SubscriptionHandle& operator=(const SubscriptionHandle&) = delete;

    SubscriptionHandle(SubscriptionHandle&& other) noexcept
        : id_(other.id_), event_type_(other.event_type_), valid_(other.valid_) {

        other.valid_ = false;
    }

    size_t GetId() const { return id_; }
    EventTypeEnum GetEventType() const { return event_type_; }
    bool IsValid() const { return valid_; }
    void Invalidate() { valid_ = false; }
};

} // namespace eerie_leap::subsys::event_bus
