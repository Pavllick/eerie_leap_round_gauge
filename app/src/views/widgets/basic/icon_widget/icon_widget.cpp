#include "views/utilitites/positioning_helpers.h"
#include "views/themes/theme_manager.h"

#include "views/widgets/basic/icons/icon_factory.h"

#include "icon_widget.h"

namespace eerie_leap::views::widgets::basic {

using namespace eerie_leap::utilities::type;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;
using namespace eerie_leap::views::widgets::basic::icons;

IconWidget::IconWidget(uint32_t id, std::shared_ptr<Frame> parent, IconType icon_type)
    : WidgetBase(id, parent), icon_type_(icon_type) {}

int IconWidget::DoRender() {
    auto lv_obj = Create();
    if(lv_obj == nullptr)
        return -1;

    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int IconWidget::ApplyTheme() {
    icon_->ApplyTheme();

    return 0;
}

lv_obj_t* IconWidget::Create() {
    if(icon_type_ == IconType::None)
        throw std::runtime_error("Invalid icon type.");

    icon_ = IconFactory::GetInstance().Create(icon_type_, configuration_, container_);
    icon_->SetAssetsManager(ui_assets_manager_);
    if(icon_->Render() != 0)
        return nullptr;
    icon_->ApplyTheme();

    lv_obj_set_x(icon_->GetContainer()->GetObject(), position_x_);
    lv_obj_set_y(icon_->GetContainer()->GetObject(), position_y_);

    return icon_->GetContainer()->GetObject();
}

void IconWidget::SetIsActive(bool is_active) {
    if(icon_ != nullptr)
        icon_->SetIsActive(is_active);
}

void IconWidget::Configure(std::shared_ptr<WidgetConfiguration> configuration) {
    WidgetBase::Configure(configuration);

    if(icon_type_ == IconType::None) {
        auto icon_type_raw = GetConfigValue<int>(
            configuration_->properties,
            WidgetProperty::GetTypeName(WidgetPropertyType::ICON_TYPE),
            0);
        icon_type_ = static_cast<IconType>(icon_type_raw);
    }

    position_x_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_X),
        0);

    position_y_ = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::POSITION_Y),
        0);

    auto event_type_raw = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::UI_EVENT_TYPE),
        0);
    auto event_type = static_cast<UiEventType>(event_type_raw);

    if(event_type != UiEventType::None) {
        auto event_subscription = UiEventBus::GetInstance().Subscribe(
            event_type,
            [this](const UiEvent& event) {
                if (auto it = event.payload.find(UiPayloadType::Value); it != event.payload.end()) {
                    if (auto* value = std::get_if<bool>(&it->second)) {
                        icon_->SetIsActive(*value);
                        ApplyTheme();
                    }
                }
            }
        );

        if(event_subscription)
            subscriptions_.push_back(std::move(*event_subscription));
    }
}

} // namespace eerie_leap::views::widgets::basic
