#pragma once

#include <vector>

#include <lvgl.h>

#include "views/widgets/basic/icon_widget/icon_widget.h"

namespace eerie_leap::views::widgets::basic {

class ArcIconWidget : public IconWidget {
private:
    float position_angle_;
    int edge_offset_px_;

    int ApplyTheme(const ITheme& theme) override;

public:
    ArcIconWidget(uint32_t id, std::shared_ptr<Frame> parent, IconType icon_type = IconType::None);
    [[nodiscard]] WidgetType GetType() const override { return WidgetType::BasicArcIcon; }

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
};

} // namespace eerie_leap::views::widgets::basic
