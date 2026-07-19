#pragma once

#include <memory>
#include <queue>

#include <zephyr/kernel.h>

#include "event.h"

namespace eerie_leap::subsys::event_bus {

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
struct EventBusTask {
    k_sem* processing_semaphore;
    std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>> subscribers;

    std::queue<Event<EventTypeEnum, PayloadTypeEnum>> event_queue;
    k_mutex queue_mutex;

    void (*dispatch_guard_before)() = nullptr;
    void (*dispatch_guard_after)() = nullptr;

    EventBusTask() {
        k_mutex_init(&queue_mutex);
    }
};

} // namespace eerie_leap::subsys::event_bus
