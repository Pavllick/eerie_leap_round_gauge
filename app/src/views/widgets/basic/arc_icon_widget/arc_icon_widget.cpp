#include "views/utilitites/positioning_helpers.h"
#include "views/themes/theme_manager.h"

#include "arc_icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

ArcIconWidget::ArcIconWidget(uint32_t id, std::shared_ptr<Frame> parent, IconType icon_type)
    : IconWidget(id, parent, icon_type) {}

int ArcIconWidget::ApplyTheme() {
    IconWidget::ApplyTheme();
    lv_obj_update_layout(container_->GetObject());

    int radius = lv_obj_get_height(container_->GetObject()) / 2;
    lv_coord_t icon_height = lv_obj_get_height(icon_->GetContainer()->GetObject());

    PositioningHelpers::PlaceObjectOnCircle(
        icon_->GetContainer()->GetObject(),
        position_x_,
        position_y_,
        radius - icon_height / 2 - edge_offset_px_,
        position_angle_);

    return 0;
}

void ArcIconWidget::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    IconWidget::Configure(configuration);

    position_angle_ = GetConfigValue<double>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE),
        0.0);

    edge_offset_px_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET),
        0);
}

} // namespace eerie_leap::views::widgets::basic
