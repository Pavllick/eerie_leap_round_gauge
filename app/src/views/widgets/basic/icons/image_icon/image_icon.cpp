#include "domain/ui_domain/models/widget_property.h"
#include "views/themes/theme_manager.h"

#include "image_icon.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::themes;

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
    container_ = std::make_shared<Frame>(Frame::Create(image).Build());

    return 0;
}

lv_obj_t* ImageIcon::Create(lv_obj_t* parent) {
    if(file_path_.empty())
        return nullptr;

    if(image_width_ <= 0 || image_height_ <= 0)
        return nullptr;

    if(ui_assets_manager_ == nullptr)
        return nullptr;

    auto image_data = ui_assets_manager_->Load(file_path_);
    if(image_data.empty())
        return nullptr;

    lv_image_ = {
        .header = {
            .magic = LV_IMAGE_HEADER_MAGIC,
            .cf = LV_COLOR_FORMAT_AL88,
            .flags = 0,
            .w = image_width_,
            .h = image_height_,
        },
        .data_size = image_data.size(),
        .data = image_data.data()
    };

    lv_obj_t* image = lv_image_create(parent_->GetObject());
    lv_image_set_src(image, &lv_image_);

    lv_obj_set_width(image, LV_SIZE_CONTENT);
    lv_obj_set_height(image, LV_SIZE_CONTENT);
    lv_obj_set_align(image, LV_ALIGN_CENTER);

    return image;
}

void ImageIcon::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    IconBase::Configure(configuration);

    file_path_ = GetConfigValue<std::pmr::string>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::FILE_PATH),
        "");

    image_width_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IMG_WIDTH),
        0);

    image_height_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IMG_HEIGHT),
        0);
}

} // namespace eerie_leap::views::widgets::basic::icons
