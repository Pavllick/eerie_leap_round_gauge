#pragma once

#include <string>

#include <lvgl.h>

#include "views/widgets/basic/icons/icon_base.h"

namespace eerie_leap::views::widgets::basic::icons {

class LabelIcon : public IconBase {
private:
    std::string label_;
    lv_obj_t* lv_label_;

    lv_obj_t* Create(lv_obj_t* parent);

public:
    explicit LabelIcon(std::shared_ptr<Frame> parent);

    int ApplyTheme() override;
    int DoRender() override;
    IconType GetIconType() const override { return IconType::Label; }
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
};

} // namespace eerie_leap::views::widgets::basic::icons
