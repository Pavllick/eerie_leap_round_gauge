#pragma once

#include <zephyr/kernel.h>

#include "subsys/threading/scoped_mutex.h"

namespace eerie_leap::views::widgets {

using eerie_leap::subsys::threading::ScopedMutex;

// EventBus::ProcessEvent copies the matching handlers out from under its
// subscriber lock and only then invokes them, so Unsubscribe() cannot cancel a
// dispatch that was already snapshotted. Handlers therefore hold a shared
// reference to this guard instead of the widget, and a dispatch that loses the
// race against destruction becomes a no-op instead of a dangling access.
//
// Lock order is always LvglLock (taken by the bus dispatch guard) then this
// lock; Detach() must never be called while holding this lock.
class WidgetDispatchGuard {
private:
    k_mutex lock_;
    void* owner_;

public:
    explicit WidgetDispatchGuard(void* owner) : owner_(owner) {
        k_mutex_init(&lock_);
    }

    WidgetDispatchGuard(const WidgetDispatchGuard&) = delete;
    WidgetDispatchGuard& operator=(const WidgetDispatchGuard&) = delete;
    WidgetDispatchGuard(WidgetDispatchGuard&&) = delete;
    WidgetDispatchGuard& operator=(WidgetDispatchGuard&&) = delete;

    template<typename TDispatch>
    void Dispatch(TDispatch&& dispatch) {
        ScopedMutex guard(lock_);

        if(owner_ != nullptr)
            dispatch();
    }

    // Blocks until an in-flight dispatch returns, every later one is a no-op.
    void Detach() {
        ScopedMutex guard(lock_);

        owner_ = nullptr;
    }
};

} // namespace eerie_leap::views::widgets
