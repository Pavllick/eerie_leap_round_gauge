#include "utilities/memory/memory_resource_manager.h"

#include "event_bus.h"

namespace eerie_leap::subsys::event_bus {

using namespace eerie_leap::utilities::memory;

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
EventBus<EventTypeEnum, PayloadTypeEnum>::EventBus(std::string bus_name, int k_stack_size)
    : bus_name_(std::move(bus_name)) {

    subscribers_ = std::make_shared<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>>();
    k_sem_init(&processing_semaphore_, 1, 1);

    work_queue_thread_ = std::make_unique<WorkQueueThread>(
        bus_name_,
        k_stack_size,
        10,
        true,
        Mrm::GetExtPmr());

    Initialize();
}

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::Initialize() {
    work_queue_thread_->Initialize();

    auto event_task = std::make_unique<EventBusTaskType>();
    event_task->processing_semaphore = &processing_semaphore_;
    event_task->subscribers = subscribers_;
    work_queue_task_ = work_queue_thread_->CreateTask(ProcessEventWork, std::move(event_task));
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
    if(work_queue_task_ && k_mutex_lock(&work_queue_task_.value().GetUserdata()->queue_mutex, K_FOREVER) == 0) {
        work_queue_task_.value().GetUserdata()->event_queue.push(event);
        k_mutex_unlock(&work_queue_task_.value().GetUserdata()->queue_mutex);

        if(!work_queue_task_.value().IsScheduled())
            work_queue_task_.value().Schedule();
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
WorkQueueTaskResult EventBus<EventTypeEnum, PayloadTypeEnum>::ProcessEventWork(EventBusTaskType* task) {
    if(k_sem_take(task->processing_semaphore, K_NO_WAIT) != 0)
        return {
            .reschedule = false
        };

    while(true) {
        std::optional<Event<EventTypeEnum, PayloadTypeEnum>> event;

        if(k_mutex_lock(&task->queue_mutex, K_FOREVER) == 0) {
            if(!task->event_queue.empty()) {
                event = std::move(task->event_queue.front());
                task->event_queue.pop();
            }

            k_mutex_unlock(&task->queue_mutex);
        }

        if(!event)
            break;

        ProcessEvent(task->subscribers, event.value());
    }

    k_sem_give(task->processing_semaphore);

    return {
        .reschedule = false
    };
}

} // namespace eerie_leap::subsys::event_bus
