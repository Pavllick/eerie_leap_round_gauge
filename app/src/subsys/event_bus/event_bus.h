#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <expected>
#include <string>
#include <unordered_map>
#include <functional>
#include <algorithm>

#include <zephyr/kernel.h>

#include "event.h"
#include "subscription.h"
#include "subscription_handle.h"
#include "event_bus_task.h"

namespace eerie_leap::subsys::event_bus {

template<EnumClassUint32 EventTypeEnum, EnumClassUint32 PayloadTypeEnum>
class EventBus {
private:
    std::string bus_name_;
    std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>> subscribers_;
    size_t next_id_ = 1;

    using EventBusTaskType = EventBusTask<EventTypeEnum, PayloadTypeEnum>;

    static constexpr int k_stack_size_ = 1024;
    static constexpr int k_priority_ = K_PRIO_COOP(10);

    k_thread_stack_t* stack_area_;
    k_work_q work_q;
    EventBusTask<EventTypeEnum, PayloadTypeEnum> event_task_;

    k_sem processing_semaphore_;
    static constexpr k_timeout_t PROCESSING_TIMEOUT = K_MSEC(200);

    void Initialize();

    static void ProcessEventWork(k_work* work);
    static void ProcessEvent(
        std::shared_ptr<std::unordered_map<EventTypeEnum, std::vector<std::unique_ptr<Subscription<EventTypeEnum, PayloadTypeEnum>>>>>& subscribers,
        const Event<EventTypeEnum, PayloadTypeEnum>& event);

protected:
    EventBus(std::string bus_name);

public:
    virtual ~EventBus();

    template<EventFilter<EventTypeEnum, PayloadTypeEnum> FilterType = AcceptAllFilter<EventTypeEnum, PayloadTypeEnum>>
    std::expected<SubscriptionHandle<EventTypeEnum>, std::string>
    Subscribe(EventTypeEnum type, FilterType filter, EventHandler<EventTypeEnum, PayloadTypeEnum> handler);

    bool Unsubscribe(SubscriptionHandle<EventTypeEnum>& handle);

    virtual void Publish(const Event<EventTypeEnum, PayloadTypeEnum>& event);
    virtual void PublishAsync(const Event<EventTypeEnum, PayloadTypeEnum>& event);
};

} // namespace eerie_leap::subsys::event_bus

#include "event_bus.tpp"
