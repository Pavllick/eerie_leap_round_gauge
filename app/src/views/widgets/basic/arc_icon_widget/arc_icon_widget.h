#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/widget_base.h"
#include "views/widgets/basic/icon_widget/icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;

class ArcIconWidget : public IconWidget {
private:
    float position_angle_;
    int edge_offset_px_;

    int ApplyTheme() override;

public:
    explicit ArcIconWidget(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::BasicArcIcon; }

    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::basic
