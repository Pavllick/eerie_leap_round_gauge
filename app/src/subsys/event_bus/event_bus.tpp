#pragma once

#include "event_bus.h"

namespace eerie_leap::subsys::event_bus {

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
EventBus<EventTypeEnum, PayloadTypeEnum>::EventBus(std::string bus_name, int k_stack_size)
    : bus_name_(std::move(bus_name)), k_stack_size_(k_stack_size) {

    subscribers_ = std::make_shared<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>>();
    k_sem_init(&processing_semaphore_, 1, 1);

    Initialize();
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
EventBus<EventTypeEnum, PayloadTypeEnum>::~EventBus() {
    k_work_queue_stop(&work_q, K_FOREVER);
    k_thread_stack_free(stack_area_);
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::Initialize() {
    stack_area_ = k_thread_stack_alloc(k_stack_size_, 0);
    k_work_queue_init(&work_q);
    k_work_queue_start(&work_q, stack_area_, k_stack_size_, k_priority_, nullptr);

    k_thread_name_set(&work_q.thread, bus_name_.c_str());

    k_work_init(&event_task_.work, ProcessEventWork);
    event_task_.processing_semaphore = &processing_semaphore_;
    event_task_.subscribers = subscribers_;
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
template<EventFilter<EventTypeEnum, PayloadTypeEnum> FilterType>
std::expected<SubscriptionHandle<EventTypeEnum>, std::string>
EventBus<EventTypeEnum, PayloadTypeEnum>::Subscribe(EventTypeEnum type, FilterType filter, EventHandler<EventTypeEnum, PayloadTypeEnum> handler) {
    try {
        size_t id = next_id_++;
        auto subscription = std::make_unique<Subscription<EventTypeEnum, PayloadTypeEnum>>(id, type, filter, std::move(handler));

        if(subscribers_->find(type) == subscribers_->end())
            subscribers_->emplace(type, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>{});
        subscribers_->at(type).push_back(std::move(subscription));

        return SubscriptionHandle<EventTypeEnum>{id, type};
    } catch (const std::exception& e) {
        return std::unexpected("Subscription failed: " + std::string(e.what()));
    }
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
bool EventBus<EventTypeEnum, PayloadTypeEnum>::Unsubscribe(SubscriptionHandle<EventTypeEnum>& handle) {
    if(!handle.IsValid())
        return false;

    auto it = subscribers_->find(handle.GetEventType());
    if(it == subscribers_->end())
        return false;

    auto& subs = it->second;
    auto sub_it = std::find_if(subs.begin(), subs.end(),
        [&handle](const auto& sub) {
            return sub->id == handle.GetId();
    });

    if(sub_it != subs.end()) {
        subs.erase(sub_it);
        handle.Invalidate();

        return true;
    }

    return false;
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::Publish(const Event<EventTypeEnum, PayloadTypeEnum>& event) {
    ProcessEvent(subscribers_, event);
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::PublishAsync(const Event<EventTypeEnum, PayloadTypeEnum>& event) {
    Event<EventTypeEnum, PayloadTypeEnum> event_copy = event;

    if(k_mutex_lock(&event_task_.queue_mutex, K_FOREVER) == 0) {
        event_task_.event_queue.push(std::move(event_copy));
        k_mutex_unlock(&event_task_.queue_mutex);

        k_work_submit_to_queue(&work_q, &event_task_.work);
    }
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::ProcessEvent(
    std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>>& subscribers,
    const Event<EventTypeEnum, PayloadTypeEnum>& event) {

    if(auto it = subscribers->find(event.type); it != subscribers->end()) {
        for(const auto& subscription : it->second) {
            if(subscription->filter(event)) {
                subscription->handler(event);
            }
        }
    }
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::ProcessEventWork(k_work* work) {
    auto* task = CONTAINER_OF(work, EventBusTaskType, work);

    if(k_sem_take(task->processing_semaphore, K_MSEC(200)) != 0)
        return;

    while(true) {
        bool has_event = false;
        Event<EventTypeEnum, PayloadTypeEnum> event;

        if(k_mutex_lock(&task->queue_mutex, K_FOREVER) == 0) {
            if(!task->event_queue.empty()) {
                event = std::move(task->event_queue.front());
                task->event_queue.pop();
                has_event = true;
            }

            k_mutex_unlock(&task->queue_mutex);
        }

        if(!has_event)
            break;

        ProcessEvent(task->subscribers, event);
    }

    k_sem_give(task->processing_semaphore);
}

} // namespace eerie_leap::subsys::event_bus
