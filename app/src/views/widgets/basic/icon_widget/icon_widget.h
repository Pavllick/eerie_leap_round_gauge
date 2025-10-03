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

    virtual void Create();
    int DoRender() override;
    void Configure(const WidgetConfiguration& config) override;
    int ApplyTheme() override;

public:
    explicit IconWidget(uint32_t id, std::shared_ptr<Frame> parent);

    WidgetType GetType() const override { return WidgetType::BasicIcon; }
};

} // namespace eerie_leap::views::widgets::basic
