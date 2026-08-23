#pragma once

#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>

#include <lvgl.h>

#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "views/renderable_base.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::widgets {

using eerie_leap::domain::ui_domain::event_bus::UiEvent;
using eerie_leap::domain::ui_domain::event_bus::UiEventBus;
using eerie_leap::domain::ui_domain::event_bus::UiEventType;
using eerie_leap::domain::ui_domain::event_bus::UiPayloadType;
using eerie_leap::domain::ui_domain::event_bus::UiSubscriptionHandle;

class WidgetBase : public IWidget, public RenderableBase {
protected:
    uint32_t id_;

    std::shared_ptr<WidgetConfiguration> configuration_;
    WidgetPosition position_px_;
    WidgetSize size_px_;

    std::shared_ptr<Frame> parent_;

    std::vector<UiSubscriptionHandle> subscriptions_;
    std::shared_ptr<AssetsManager> ui_assets_manager_ = nullptr;

    bool is_active_ = false;

    int SetVisibility(bool is_visible);

    // Subscribes and drops events while the widget is unrendered or its group is hidden,
    // so a widget nobody can see never repaints or animates.
    template<typename FilterType>
    void SubscribeWhileActive(UiEventType type, FilterType filter, std::function<void(const UiEvent&)> handler) {
        auto subscription = UiEventBus::GetInstance().Subscribe(
            type,
            std::move(filter),
            [this, handler = std::move(handler)](const UiEvent& event) {
                if(IsReady() && IsActive())
                    handler(event);
            });

        if(subscription)
            subscriptions_.push_back(std::move(*subscription));
    }

    void SubscribeWhileActive(UiEventType type, std::function<void(const UiEvent&)> handler) {
        SubscribeWhileActive(
            type,
            eerie_leap::subsys::event_bus::AcceptAllFilter<UiEventType, UiPayloadType>{ },
            std::move(handler));
    }

public:
    WidgetBase(uint32_t id, std::shared_ptr<Frame> parent);
    ~WidgetBase() override;

    uint32_t GetId() const override;
    bool IsSmoothed() const override;
    bool IsVisible() const override;
    bool IsActive() const;

    void OnActivated() override;
    void OnDeactivated() override;

    void SetAssetsManager(std::shared_ptr<AssetsManager> ui_assets_manager) override;
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
    std::shared_ptr<WidgetConfiguration> GetConfiguration() const override;

    WidgetPosition GetPositionPx() const override;
    void SetPositionPx(const WidgetPosition& pos) override;
    WidgetSize GetSizePx() const override;
    void SetSizePx(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets
