#include "domain/ui_domain/models/widget_property.h"
#include "views/themes/theme_manager.h"
#include "views/assets/image/image_register.h"

#include "image_icon.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::themes;
using namespace eerie_leap::views::assets::image;

ImageIcon::ImageIcon(std::shared_ptr<Frame> parent) : IconBase(std::move(parent)) {}

int ImageIcon::ApplyTheme() {
    auto theme = ThemeManager::GetInstance().GetCurrentTheme();

    lv_obj_set_style_opa(
        container_->GetObject(),
        is_active_ ? LV_OPA_COVER : 0,
        0);

    return 0;
}

int ImageIcon::DoRender() {
    lv_obj_t* image = Create(parent_->GetObject());
    lv_image_set_src(image, &ui_img_norma_al88);

    container_ = std::make_shared<Frame>(Frame::Create(image).Build());

    return 0;
}

lv_obj_t* ImageIcon::Create(lv_obj_t* parent) {
    lv_obj_t* image = lv_image_create(parent_->GetObject());
    lv_obj_set_width(image, LV_SIZE_CONTENT);
    lv_obj_set_height(image, LV_SIZE_CONTENT);
    lv_obj_set_align(image, LV_ALIGN_CENTER);

    return image;
}

void ImageIcon::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    IconBase::Configure(configuration);

    // file_path_ = GetConfigValue<std::pmr::string>(
    //     configuration_->properties,
    //     WidgetProperty::GetTypeName(WidgetPropertyType::FILE_PATH),
    //     "");
}

} // namespace eerie_leap::views::widgets::basic::icons
