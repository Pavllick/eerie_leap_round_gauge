#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <unordered_set>
#include <utility>
#include <vector>

#include <lvgl.h>

#include "subsys/event_bus/event_channel.h"

#include "domain/ui_domain/lvgl_lock.h"
#include "domain/ui_domain/models/property_binding.h"

#include "views/renderable_base.h"
#include "views/widgets/i_widget.h"
#include "views/widgets/widget_context.h"
#include "views/widgets/widget_dispatch_guard.h"
#include "views/widgets/widget_property_store.h"

namespace eerie_leap::views::widgets {

using eerie_leap::domain::ui_domain::ScopedLvglLock;
using eerie_leap::domain::ui_domain::models::PropertyBinding;
using eerie_leap::subsys::event_bus::AnySubscription;
using eerie_leap::subsys::event_bus::ErasedEventFilter;
using eerie_leap::subsys::event_bus::ErasedPayload;
using eerie_leap::subsys::event_bus::ErasedPayloadView;
using eerie_leap::subsys::event_bus::EventData;
using eerie_leap::subsys::event_bus::IEventChannel;

class WidgetBase : public IWidget, public RenderableBase {
protected:
    // A binding that writes back. Resolved once at configure time so user input costs a scan of a
    // handful of entries rather than a registry lookup.
    struct OutboundBinding {
        WidgetPropertyType target;
        IEventChannel* channel;
        uint32_t event_type;
        uint32_t payload_key;
        uint32_t selector_key;
        std::optional<EventData> selector_value;
    };

    uint32_t id_;

    std::shared_ptr<WidgetConfiguration> configuration_;
    std::shared_ptr<WidgetPropertyStore> properties_;
    WidgetPosition position_px_;
    WidgetSize size_px_;

    std::shared_ptr<Frame> parent_;

    std::vector<AnySubscription> subscriptions_;
    std::vector<OutboundBinding> outbound_bindings_;
    std::shared_ptr<WidgetDispatchGuard> dispatch_guard_;
    WidgetContext context_;

    bool is_active_ = false;

    int SetVisibility(bool is_visible);

    void AddSubscription(AnySubscription subscription);

    // Resolves configuration_->bindings against EventChannelRegistry. A binding naming an unknown
    // channel or an unsupported property is dropped with a warning, never fatal: configuration
    // outlives the code that reads it.
    void ResolveBindings();

    // User input. Unlike an inbound binding this publishes on the property's outbound bindings,
    // which is the whole of the echo suppression rule - inbound never publishes, so there is no
    // loop back through the owner.
    void SetPropertyLocal(WidgetPropertyType type, const ConfigValue& value);

    // Declares what this widget understands, base class first. A derived override calls its base
    // before adding its own, so the replay below applies base properties first.
    virtual void RegisterProperties(WidgetPropertyStore& store);

    // Reacts to one property. A derived override handles its own keys and delegates the rest.
    // Runs before the LVGL objects exist, so it may only touch members and the container.
    virtual void OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value);

    // One-shot setup after every property has been applied, for work that must not repeat when a
    // property changes again - subscriptions above all.
    virtual void OnConfigured();

    // Reacts to a value already written to the store. The caller holds the LVGL lock and the
    // dispatch guard, in that order.
    void NotifyPropertyChanged(WidgetPropertyType type, const ConfigValue& value, PropertyChangeEffect effect);

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
