#pragma once

#include <memory>
#include <lvgl.h>

#include "views/i_renderable.h"
#include "domain/ui_domain/models/widget_type.h"
#include "domain/ui_domain/models/widget_property.h"
#include "domain/ui_domain/models/widget_position.h"
#include "domain/ui_domain/models/widget_size.h"
#include "domain/ui_domain/models/widget_configuration.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views;

class IWidget : public virtual IRenderable {
public:
    virtual ~IWidget() = default;

    virtual WidgetType GetType() const = 0;
    virtual uint32_t GetId() const = 0;
    virtual bool IsVisible() const = 0;

    // Configuration
    virtual void Configure(std::shared_ptr<WidgetConfiguration> configuration) = 0;
    virtual std::shared_ptr<WidgetConfiguration> GetConfiguration() const = 0;
    virtual bool IsAnimated() const = 0;

    // Layout
    virtual WidgetPosition GetPositionPx() const = 0;
    virtual void SetPositionPx(const WidgetPosition& pos) = 0;
    virtual WidgetSize GetSizePx() const = 0;
    virtual void SetSizePx(const WidgetSize& size) = 0;
};

} // namespace eerie_leap::views::widgets
