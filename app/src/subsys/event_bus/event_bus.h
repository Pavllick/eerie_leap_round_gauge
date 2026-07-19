#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>
#include <expected>
#include <string>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <zephyr/kernel.h>

#include "subsys/threading/work_queue_thread.h"

#include "event.h"
#include "subscription.h"
#include "subscription_handle.h"
#include "event_bus_task.h"

namespace eerie_leap::subsys::event_bus {

namespace threading = eerie_leap::subsys::threading;
namespace concepts = eerie_leap::utilities::concepts;

using threading::WorkQueueThread;

template<concepts::EnumClassUint32 EventTypeEnum, concepts::EnumClassUint32 PayloadTypeEnum>
class EventBus {
public:
    using DispatchGuardFn = void (*)();

private:
    std::string bus_name_;
    std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>> subscribers_;
    size_t next_id_ = 1;

    using EventBusTaskType = EventBusTask<EventTypeEnum, PayloadTypeEnum>;

    std::unique_ptr<WorkQueueThread> work_queue_thread_;
    std::optional<threading::WorkQueueTask<EventBusTaskType>> work_queue_task_;

    k_sem processing_semaphore_;
    static constexpr k_timeout_t PROCESSING_TIMEOUT = K_MSEC(200);

    // Optional hooks invoked around subscriber dispatch for custom behavior before and after event handling
    DispatchGuardFn dispatch_guard_before_ = nullptr;
    DispatchGuardFn dispatch_guard_after_ = nullptr;

    void Initialize();

    static threading::WorkQueueTaskResult ProcessEventWork(EventBusTaskType* task);
    static void ProcessEvent(
        std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>>& subscribers,
        const Event<EventTypeEnum, PayloadTypeEnum>& event,
        DispatchGuardFn dispatch_guard_before,
        DispatchGuardFn dispatch_guard_after);

protected:
    EventBus(
        std::string bus_name,
        int k_stack_size,
        DispatchGuardFn dispatch_guard_before = nullptr,
        DispatchGuardFn dispatch_guard_after = nullptr);

public:
    virtual ~EventBus() = default;

    template<EventFilter<EventTypeEnum, PayloadTypeEnum> FilterType = AcceptAllFilter<EventTypeEnum, PayloadTypeEnum>>
    std::expected<SubscriptionHandle<EventTypeEnum>, std::string>
    Subscribe(EventTypeEnum type, FilterType filter, EventHandler<EventTypeEnum, PayloadTypeEnum> handler);

    std::expected<SubscriptionHandle<EventTypeEnum>, std::string>
    Subscribe(EventTypeEnum type, EventHandler<EventTypeEnum, PayloadTypeEnum> handler) {
        return Subscribe(type, AcceptAllFilter<EventTypeEnum, PayloadTypeEnum>{ }, std::move(handler));
    }

    bool Unsubscribe(SubscriptionHandle<EventTypeEnum>& handle);

    virtual void Publish(const Event<EventTypeEnum, PayloadTypeEnum>& event);
    virtual void PublishAsync(const Event<EventTypeEnum, PayloadTypeEnum>& event);
};

} // namespace eerie_leap::subsys::event_bus

#include "event_bus.tpp"
