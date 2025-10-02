#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/widget_base.h"
#include "views/widgets/basic/label_widget/label_icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;

class ArcLabelIconWidget : public LabelIconWidget {
private:
    float position_angle_;
    int edge_offset_px_;

    int ApplyTheme() override;

public:
    explicit ArcLabelIconWidget(uint32_t id, std::shared_ptr<Frame> parent);
    WidgetType GetType() const override { return WidgetType::BasicArcLabelIcon; }

    void Configure(const WidgetConfiguration& config) override;
};

} // namespace eerie_leap::views::widgets::basic
