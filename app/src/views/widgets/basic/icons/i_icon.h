#pragma once

#include <lvgl.h>

#include "subsys/assets/assets_manager.h"
#include "domain/ui_domain/models/widget_configuration.h"
#include "domain/ui_domain/models/icon_type.h"

#include "views/i_renderable.h"

namespace eerie_leap::views::widgets::basic::icons {

using eerie_leap::subsys::assets::AssetsManager;
using eerie_leap::domain::ui_domain::models::WidgetConfiguration;
using eerie_leap::domain::ui_domain::models::IconType;

using eerie_leap::views::utilitites::Frame;
using eerie_leap::views::IRenderable;

class IIcon : public virtual IRenderable {
public:
    virtual ~IIcon() = default;

    virtual void SetAssetsManager(std::shared_ptr<AssetsManager> ui_assets_manager) = 0;

    virtual IconType GetIconType() const = 0;
    virtual void Configure(std::shared_ptr<WidgetConfiguration> configuration) = 0;
    virtual void SetIsActive(bool is_active) = 0;
};

} // namespace eerie_leap::views::widgets::basic::icons
