#include "icon_base.h"

#include "domain/ui_domain/models/widget_property.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;

IconBase::IconBase(std::shared_ptr<Frame> parent) : parent_(std::move(parent)), is_active_(false) { }

void IconBase::SetIsActive(bool is_active) {
    is_active_ = is_active;
}

void IconBase::Configure(const WidgetConfiguration& config) {
    configuration_ = config;

    is_active_ = GetConfigValue<bool>(
        configuration_.properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE),
        false);
}

} // namespace eerie_leap::views::widgets::basic::icons
