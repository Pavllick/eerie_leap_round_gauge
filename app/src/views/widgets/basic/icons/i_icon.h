#pragma once

#include <lvgl.h>

#include "domain/ui_domain/models/widget_configuration.h"
#include "domain/ui_domain/models/icon_type.h"
#include "views/i_renderable.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views;

class IIcon : public virtual IRenderable {
public:
    virtual ~IIcon() = default;

    virtual IconType GetIconType() const = 0;
    virtual void Configure(std::shared_ptr<WidgetConfiguration> configuration) = 0;
    virtual void SetIsActive(bool is_active) = 0;
};

} // namespace eerie_leap::views::widgets::basic::icons
