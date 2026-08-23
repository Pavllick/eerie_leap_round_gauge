#pragma once

#include <memory>
#include <lvgl.h>

#include "subsys/assets/assets_manager.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_position.h"
#include "domain/ui_domain/models/widget_size.h"
#include "domain/ui_domain/models/widget_configuration.h"
#include "views/i_renderable.h"

namespace eerie_leap::views::widgets {

using eerie_leap::subsys::assets::AssetsManager;
using eerie_leap::domain::ui_domain::models::WidgetConfiguration;
using eerie_leap::domain::ui_domain::models::WidgetPosition;
using eerie_leap::domain::ui_domain::models::WidgetSize;
using eerie_leap::domain::ui_domain::models::WidgetType;

class IWidget : public virtual IRenderable {
public:
    virtual ~IWidget() = default;

    virtual WidgetType GetType() const = 0;
    virtual uint32_t GetId() const = 0;
    virtual bool IsVisible() const = 0;

    // Lifecycle - a widget on a hidden screen group must not animate or repaint.
    virtual void OnActivated() = 0;
    virtual void OnDeactivated() = 0;

    // Configuration
    virtual void SetAssetsManager(std::shared_ptr<AssetsManager> ui_assets_manager) = 0;
    virtual void Configure(std::shared_ptr<WidgetConfiguration> configuration) = 0;
    virtual std::shared_ptr<WidgetConfiguration> GetConfiguration() const = 0;
    virtual bool IsSmoothed() const = 0;

    // Layout
    virtual WidgetPosition GetPositionPx() const = 0;
    virtual void SetPositionPx(const WidgetPosition& pos) = 0;
    virtual WidgetSize GetSizePx() const = 0;
    virtual void SetSizePx(const WidgetSize& size) = 0;
};

} // namespace eerie_leap::views::widgets
