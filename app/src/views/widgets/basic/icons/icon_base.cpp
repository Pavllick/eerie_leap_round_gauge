#include "icon_base.h"

#include "domain/ui_domain/models/widget_property.h"
#include "views/themes/theme_manager.h"

namespace eerie_leap::views::widgets::basic::icons {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using eerie_leap::views::themes::ThemeManager;

IconBase::IconBase(std::shared_ptr<Frame> parent)
    : parent_(std::move(parent)), is_active_(false) {

    container_ = std::make_shared<Frame>(Frame::CreateWrapped(parent_->GetObject())
        .SetWidth(100, false)
        .SetHeight(100, false)
        .Build());
}

void IconBase::SetAssetsManager(std::shared_ptr<AssetsManager> ui_assets_manager) {
    ui_assets_manager_ = std::move(ui_assets_manager);
}

void IconBase::SetIsActive(bool is_active) {
    is_active_ = is_active;

    if(!is_ready_)
        return;

    ApplyTheme(ThemeManager::GetInstance().GetCurrentTheme());
    container_->Invalidate();
}

void IconBase::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    configuration_ = std::move(configuration);

    is_active_ = GetConfigValue<bool>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::IS_ACTIVE),
        true);
}

} // namespace eerie_leap::views::widgets::basic::icons
