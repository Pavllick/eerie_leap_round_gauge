#pragma once

#include <memory>
#include <queue>

#include <zephyr/kernel.h>

#include "event.h"

namespace eerie_leap::subsys::event_bus {

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
struct EventBusTask {
    k_work work;
    k_sem* processing_semaphore;
    std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>> subscribers;

    std::queue<Event<EventTypeEnum, PayloadTypeEnum>> event_queue;
    k_mutex queue_mutex;

    EventBusTask() {
        k_mutex_init(&queue_mutex);
    }
};

} // namespace eerie_leap::subsys::event_bus
