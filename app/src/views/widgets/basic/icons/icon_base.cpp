#include "icon_base.h"

#include "domain/ui_domain/models/widget_property.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

IconBase::IconBase(std::shared_ptr<Frame> parent)
    : parent_(std::move(parent)), is_active_(false), is_animated_(false) { }

void IconBase::SetIsActive(bool is_active) {
    is_active_ = is_active;
}

void IconBase::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    configuration_ = std::move(configuration);

    is_active_ = GetConfigValue<bool>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE),
        false);

    is_animated_ = GetConfigValue<bool>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_ANIMATED),
        false);
}

} // namespace eerie_leap::views::widgets::basic::icons
