#pragma once

#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include <lvgl.h>

#include "subsys/event_bus/scoped_subscription.h"

#include "domain/ui_domain/lvgl_lock.h"

#include "views/renderable_base.h"
#include "views/widgets/i_widget.h"
#include "views/widgets/widget_context.h"
#include "views/widgets/widget_dispatch_guard.h"

namespace eerie_leap::views::widgets {

using eerie_leap::domain::ui_domain::ScopedLvglLock;
using eerie_leap::subsys::event_bus::AcceptAllFilter;
using eerie_leap::subsys::event_bus::AnySubscription;
using eerie_leap::subsys::event_bus::CreateScopedSubscription;

class WidgetBase : public IWidget, public RenderableBase {
protected:
    uint32_t id_;

    std::shared_ptr<WidgetConfiguration> configuration_;
    WidgetPosition position_px_;
    WidgetSize size_px_;

    std::shared_ptr<Frame> parent_;

    std::vector<AnySubscription> subscriptions_;
    std::shared_ptr<WidgetDispatchGuard> dispatch_guard_;
    WidgetContext context_;

    bool is_active_ = false;

    int SetVisibility(bool is_visible);

    // Wraps a callback so it drops while the widget is unrendered or its group is
    // hidden, and so a dispatch racing destruction becomes a no-op.
    //
    // LvglLock before the dispatch guard: ~WidgetBase runs under the LVGL lock and
    // takes the guard second, so the reverse order deadlocks.
    template<typename THandler>
    auto GuardWhileActive(THandler handler) {
        return [guard = dispatch_guard_, this, handler = std::move(handler)](auto&&... args) {
            ScopedLvglLock lvgl_guard;

            guard->Dispatch([&] {
                if(IsReady() && IsActive())
                    handler(std::forward<decltype(args)>(args)...);
            });
        };
    }

    // Subscribes and drops events while the widget is unrendered or its group is hidden,
    // so a widget nobody can see never repaints or animates.
    template<typename ChannelType, typename FilterType>
    void SubscribeWhileActive(
        ChannelType& channel,
        typename ChannelType::EventTypeEnum type,
        FilterType filter,
        std::function<void(const typename ChannelType::EventMessage&)> handler) {

        AddSubscription(CreateScopedSubscription(
            channel,
            type,
            std::move(filter),
            GuardWhileActive(std::move(handler))));
    }

    template<typename ChannelType>
    void SubscribeWhileActive(
        ChannelType& channel,
        typename ChannelType::EventTypeEnum type,
        std::function<void(const typename ChannelType::EventMessage&)> handler) {

        SubscribeWhileActive(
            channel,
            type,
            AcceptAllFilter<typename ChannelType::EventTypeEnum, typename ChannelType::PayloadTypeEnum>{ },
            std::move(handler));
    }

    void AddSubscription(AnySubscription subscription);

    // Every destructor that runs before ~WidgetBase must call this first, or a
    // dispatch already in flight can reach members that have just been destroyed.
    void DetachDispatch();

public:
    WidgetBase(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context);
    ~WidgetBase() override;

    uint32_t GetId() const override;
    bool IsSmoothed() const override;
    bool IsVisible() const override;
    bool IsActive() const;

    void OnActivated() override;
    void OnDeactivated() override;

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
    std::shared_ptr<WidgetConfiguration> GetConfiguration() const override;

    WidgetPosition GetPositionPx() const override;
    void SetPositionPx(const WidgetPosition& pos) override;
    WidgetSize GetSizePx() const override;
    void SetSizePx(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets
