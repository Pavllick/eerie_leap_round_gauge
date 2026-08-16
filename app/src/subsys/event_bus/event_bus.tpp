#include <zephyr/sys/printk.h>

#include "utilities/memory/memory_resource_manager.h"

#include "event_bus.h"

namespace eerie_leap::subsys::event_bus {

using eerie_leap::utilities::memory::Mrm;

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
EventBus<EventTypeEnum, PayloadTypeEnum>::EventBus(
    std::string bus_name,
    int k_stack_size,
    DispatchGuardFn dispatch_guard_before,
    DispatchGuardFn dispatch_guard_after)
        : bus_name_(
            std::move(bus_name)),
            dispatch_guard_before_(dispatch_guard_before),
            dispatch_guard_after_(dispatch_guard_after) {

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

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::Initialize() {
    work_queue_thread_->Initialize();

    auto event_task = std::make_unique<EventBusTaskType>();
    event_task->processing_semaphore = &processing_semaphore_;
    event_task->subscribers = subscribers_;
    event_task->dispatch_guard_before = dispatch_guard_before_;
    event_task->dispatch_guard_after = dispatch_guard_after_;
    work_queue_task_ = work_queue_thread_->CreateTask(ProcessEventWork, std::move(event_task));
}

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
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

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
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

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::Publish(const Event<EventTypeEnum, PayloadTypeEnum>& event) {
    ProcessEvent(subscribers_, event, dispatch_guard_before_, dispatch_guard_after_);
}

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::PublishAsync(const Event<EventTypeEnum, PayloadTypeEnum>& event) {
    if(work_queue_task_ && k_mutex_lock(&work_queue_task_.value().GetUserdata()->queue_mutex, K_FOREVER) == 0) {
        work_queue_task_.value().GetUserdata()->event_queue.push(event);
        k_mutex_unlock(&work_queue_task_.value().GetUserdata()->queue_mutex);

        if(!work_queue_task_.value().IsScheduled())
            work_queue_task_.value().Schedule();
    }
}

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
void EventBus<EventTypeEnum, PayloadTypeEnum>::ProcessEvent(
    std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>>& subscribers,
    const Event<EventTypeEnum, PayloadTypeEnum>& event,
    DispatchGuardFn dispatch_guard_before,
    DispatchGuardFn dispatch_guard_after) {

    if(dispatch_guard_before)
        dispatch_guard_before();

    struct GuardRelease {
        DispatchGuardFn fn;
        ~GuardRelease() { if(fn) fn(); }
    } release_guard{dispatch_guard_after};

    try {
        if(auto it = subscribers->find(event.type); it != subscribers->end()) {
            for(const auto& subscription : it->second) {
                if(subscription->filter(event)) {
                    subscription->handler(event);
                }
            }
        }
    } catch (const std::exception& e) {
        printk("[event_bus] subscriber handler threw for event type %u: %s\n",
            static_cast<unsigned>(event.type), e.what());
    } catch (...) {
        printk("[event_bus] subscriber handler threw a non-standard exception for event type %u\n",
            static_cast<unsigned>(event.type));
    }
}

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
threading::WorkQueueTaskResult EventBus<EventTypeEnum, PayloadTypeEnum>::ProcessEventWork(EventBusTaskType* task) {
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

        ProcessEvent(task->subscribers, event.value(), task->dispatch_guard_before, task->dispatch_guard_after);
    }

    k_sem_give(task->processing_semaphore);

    return {
        .reschedule = false
    };
}

} // namespace eerie_leap::subsys::event_bus
