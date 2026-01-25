#pragma once

#include <memory>

#include <lvgl.h>

#include "views/widgets/basic/icons/i_icon.h"
#include "views/widgets/widget_base.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::basic::icons;

class IconWidget : public WidgetBase {
protected:
    IconType icon_type_;
    int position_x_;
    int position_y_;

    std::unique_ptr<IIcon> icon_;

    virtual lv_obj_t* Create();
    int DoRender() override;

public:
    IconWidget(uint32_t id, std::shared_ptr<Frame> parent, IconType icon_type = IconType::None);

    void SetIsActive(bool is_active);

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
    int ApplyTheme(const ITheme& theme) override;

    WidgetType GetType() const override { return WidgetType::BasicIcon; }
};

} // namespace eerie_leap::views::widgets::basic
