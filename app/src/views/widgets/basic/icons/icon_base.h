#pragma once

#include <lvgl.h>
#include <memory>

#include "domain/ui_domain/models/widget_configuration.h"
#include "views/renderable_base.h"
#include "views/utilitites/frame.h"

#include "i_icon.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views;

class IconBase : public RenderableBase, public IIcon {
protected:
    std::shared_ptr<Frame> parent_;
    std::shared_ptr<WidgetConfiguration> configuration_;
    bool is_active_;
    bool is_smoothed_;

public:
    explicit IconBase(std::shared_ptr<Frame> parent);
    virtual ~IconBase() = default;

    void SetIsActive(bool is_active) override;
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
};

} // namespace eerie_leap::views::widgets::basic::icons
