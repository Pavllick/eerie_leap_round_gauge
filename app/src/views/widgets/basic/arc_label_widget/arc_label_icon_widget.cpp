#include "views/utilitites/positioning_helpers.h"
#include "views/themes/theme_manager.h"

#include "arc_label_icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

ArcLabelIconWidget::ArcLabelIconWidget(uint32_t id, std::shared_ptr<Frame> parent)
    : LabelIconWidget(id, parent) { }

int ArcLabelIconWidget::ApplyTheme() {
    LabelIconWidget::ApplyTheme();
    lv_obj_update_layout(lv_icon_);

    int radius = lv_obj_get_height(container_->GetObject()) / 2;
    lv_coord_t icon_height = lv_obj_get_height(lv_icon_);

    PositioningHelpers::PlaceObjectOnCircle(
        lv_icon_,
        position_x_,
        position_y_,
        radius - icon_height / 2 - edge_offset_px_,
        position_angle_);

    return 0;
}

void ArcLabelIconWidget::Configure(const WidgetConfiguration& config) {
    LabelIconWidget::Configure(config);

    position_angle_ = GetConfigValue<double>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_ANGLE),
        0.0);

    edge_offset_px_ = GetConfigValue<int>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::EDGE_OFFSET),
        0);
}

} // namespace eerie_leap::views::widgets::basic
