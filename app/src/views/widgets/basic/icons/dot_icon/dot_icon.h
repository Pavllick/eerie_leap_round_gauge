#pragma once

#include <string>

#include <lvgl.h>

#include "views/widgets/basic/icons/icon_base.h"

namespace eerie_leap::views::widgets::basic::icons {

class DotIcon : public IconBase {
private:
    lv_anim_t animation_;

    static void AnimateIconCallback(void* obj, int32_t value);

    lv_obj_t* Create(lv_obj_t* parent);
    lv_anim_t CreateAnimation();

public:
    explicit DotIcon(std::shared_ptr<Frame> parent);

    int ApplyTheme() override;
    int DoRender() override;
    IconType GetIconType() const override { return IconType::Dot; }
};

} // namespace eerie_leap::views::widgets::basic::icons
