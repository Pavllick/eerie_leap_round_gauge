#pragma once

#include <memory>
#include <unordered_set>

#include <lvgl.h>

#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "views/renderable_base.h"
#include "views/utilitites/frame.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::domain::ui_domain::event_bus;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::domain::ui_domain::assets_manager;

using namespace eerie_leap::views;
using namespace eerie_leap::views::utilitites;

class WidgetBase : public IWidget, public RenderableBase {
protected:
    uint32_t id_;

    std::shared_ptr<WidgetConfiguration> configuration_;
    WidgetPosition position_px_;
    WidgetSize size_px_;

    std::shared_ptr<Frame> parent_;

    std::vector<UiSubscriptionHandle> subscriptions_;
    std::shared_ptr<UiAssetsManager> ui_assets_manager_ = nullptr;

    int SetVisibility(bool is_visible);

public:
    WidgetBase(uint32_t id, std::shared_ptr<Frame> parent);
    ~WidgetBase() override;

    uint32_t GetId() const override;
    bool IsSmoothed() const override;
    bool IsVisible() const override;

    void SetAssetsManager(std::shared_ptr<UiAssetsManager> ui_assets_manager) override;
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
    std::shared_ptr<WidgetConfiguration> GetConfiguration() const override;

    WidgetPosition GetPositionPx() const override;
    void SetPositionPx(const WidgetPosition& pos) override;
    WidgetSize GetSizePx() const override;
    void SetSizePx(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets
