#pragma once

#include <lvgl.h>

#include "views/widgets/configuration/widget_type.h"
#include "views/widgets/configuration/widget_position.h"
#include "views/widgets/configuration/widget_size.h"
#include "views/widgets/configuration/widget_configuration.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::views::widgets::configuration;

class IWidget {
public:
    virtual ~IWidget() = default;

    virtual int Render() = 0;
    virtual WidgetType GetType() const = 0;
    virtual uint32_t GetId() const = 0;

    // Configuration
    virtual void Configure(const WidgetConfiguration& config) = 0;
    virtual WidgetConfiguration GetConfiguration() const = 0;
    virtual bool IsAnimationEnabled() const = 0;

    // Layout
    virtual WidgetPosition GetPositionPx() const = 0;
    virtual void SetPositionPx(const WidgetPosition& pos) = 0;
    virtual WidgetSize GetSizePx() const = 0;
    virtual void SetSizePx(const WidgetSize& size) = 0;
};

} // namespace eerie_leap::views::widgets
