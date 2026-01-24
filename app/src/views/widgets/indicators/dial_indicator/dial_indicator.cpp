#include <zephyr/kernel.h>

#include "views/widgets/indicators/indicator_base.h"
#include "views/themes/theme_manager.h"

#include "dial_indicator.h"

namespace eerie_leap::views::widgets::indicators {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

DialIndicator::DialIndicator(uint32_t id, std::shared_ptr<Frame> parent)
    : IndicatorBase(id, parent) {}

int DialIndicator::DoRender() {
    auto lv_obj = Create(static_cast<int32_t>(range_start_), static_cast<int32_t>(range_end_));
    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int DialIndicator::ApplyTheme() {
    needle_icon_->ApplyTheme();

    return 0;
}

lv_obj_t* DialIndicator::Create(int32_t range_start, int32_t range_end) {
    needle_icon_ = std::make_unique<IconWidget>(id_, container_, IconType::Image);
    needle_icon_->Configure(configuration_);
    needle_icon_->SetAssetsManager(ui_assets_manager_);
    needle_icon_->Render();

    lv_needle_icon_ = needle_icon_->GetContainer()->GetChild()->GetObject();
    UpdateIndicator(start_angle_);

    return needle_icon_->GetContainer()->GetObject();
}

uint32_t DialIndicator::GetAngleForValue(float value) {
    float resolution = (abs(end_angle_ - start_angle_) * 10) / abs(range_end_ - range_start_);
    float angle_delta = resolution * value;

    return start_angle_ * 10 - 1800 + static_cast<uint32_t>(angle_delta);
}

void DialIndicator::UpdateIndicator(float value) {
    lv_image_set_rotation(
        lv_needle_icon_,
        GetAngleForValue(value));

    // NOTE: lv_image_set_pivot is meant to work along with lv_image_set_rotation,
    // but it does not work as expected out of the box. There is a patch applyed to LVGL
    // in order to fix that bug. Transform is another option here, but it adds artifacts
    // around an image when rotated.
    // lv_obj_set_style_transform_rotation(
    //     lv_needle_icon_,
    //     GetAngleForValue(value),
    //     LV_PART_MAIN | LV_STATE_DEFAULT);
}

void DialIndicator::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    IndicatorBase::Configure(configuration);

    start_angle_ = GetConfigValue<int>(
        configuration->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::START_ANGLE),
        DEFAULT_START_ANGLE);

    end_angle_ = GetConfigValue<int>(
        configuration->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::END_ANGLE),
        DEFAULT_END_ANGLE);
}

} // namespace eerie_leap::views::widgets::indicators
