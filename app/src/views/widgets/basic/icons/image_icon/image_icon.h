#pragma once

#include <string>

#include <lvgl.h>

#include "views/widgets/basic/icons/icon_base.h"

namespace eerie_leap::views::widgets::basic::icons {

class ImageIcon : public IconBase {
private:
    std::pmr::string file_path_;
    int image_width_ = 0;
    int image_height_ = 0;
    lv_image_dsc_t lv_image_;

    lv_obj_t* Create(lv_obj_t* parent);

public:
    explicit ImageIcon(std::shared_ptr<Frame> parent);
    virtual ~ImageIcon() = default;

    int ApplyTheme() override;
    int DoRender() override;
    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;

    IconType GetIconType() const override { return IconType::Image; }
};

} // namespace eerie_leap::views::widgets::basic::icons
