#pragma once

#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include <lvgl.h>

#include "subsys/event_bus/event_channel.h"

#include "domain/ui_domain/lvgl_lock.h"

#include "views/renderable_base.h"
#include "views/widgets/i_widget.h"
#include "views/widgets/widget_context.h"
#include "views/widgets/widget_dispatch_guard.h"
#include "views/widgets/widget_property_store.h"

namespace eerie_leap::views::widgets {

using eerie_leap::domain::ui_domain::ScopedLvglLock;
using eerie_leap::subsys::event_bus::AcceptAllFilter;
using eerie_leap::subsys::event_bus::AnySubscription;
using eerie_leap::subsys::event_bus::CreateScopedSubscription;

class WidgetBase : public IWidget, public RenderableBase {
protected:
    uint32_t id_;

    std::shared_ptr<WidgetConfiguration> configuration_;
    std::shared_ptr<WidgetPropertyStore> properties_;
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

    // Declares what this widget understands, base class first. A derived override calls its base
    // before adding its own, so the replay below applies base properties first.
    virtual void RegisterProperties(WidgetPropertyStore& store);

    // Reacts to one property. A derived override handles its own keys and delegates the rest.
    // Runs before the LVGL objects exist, so it may only touch members and the container.
    virtual void OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value);

    // One-shot setup after every property has been applied, for work that must not repeat when a
    // property changes again - subscriptions above all.
    virtual void OnConfigured();

    // Inbound value from an event. Always updates the store, even while the widget is hidden or
    // unrendered, so it never goes stale; only the effect is guarded and deferred.
    void ApplyProperty(WidgetPropertyType type, const ConfigValue& value);

    // Applies the current stored value of every registered property and runs the strongest effect
    // once. Callers are already on the UI thread, so this skips the dispatch guard.
    void ReplayProperties();

    void RunEffect(PropertyChangeEffect effect);

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

    // Sealed: the configuration path is register -> seed -> replay -> OnConfigured, and a derived
    // override would run its own reads after the replay had already used the defaults.
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) final;
    std::shared_ptr<WidgetConfiguration> GetConfiguration() const override;

    WidgetPosition GetPositionPx() const override;
    void SetPositionPx(const WidgetPosition& pos) override;
    WidgetSize GetSizePx() const override;
    void SetSizePx(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets
