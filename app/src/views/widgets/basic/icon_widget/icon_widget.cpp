#include <variant>

#include "domain/ui_domain/event_bus/ui_signal_channel.h"
#include "domain/ui_domain/models/widget_property.h"

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

using eerie_leap::domain::ui_domain::event_bus::UiSignalType;
using eerie_leap::domain::ui_domain::event_bus::UiSignalChannel;
using eerie_leap::domain::ui_domain::event_bus::UiSignalPayloadType;

IconWidget::IconWidget(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context, IconType icon_type)
    : WidgetBase(id, std::move(parent), std::move(context)), icon_type_(icon_type) {}

int IconWidget::DoRender() {
    auto lv_obj = Create();
    if(lv_obj == nullptr)
        return -1;

    auto child = std::make_shared<Frame>(
        Frame::Create(lv_obj).Build());
    container_->SetChild(child);

    return 0;
}

int IconWidget::ApplyTheme(const ITheme& theme) {
    icon_->ApplyTheme(theme);

    return 0;
}

lv_obj_t* IconWidget::Create() {
    if(icon_type_ == IconType::None)
        throw std::runtime_error("Invalid icon type.");

    icon_ = IconFactory::GetInstance().Create(icon_type_, configuration_, container_);
    icon_->SetAssetsManager(context_.assets_manager);
    if(icon_->Render() != 0)
        return nullptr;

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

    auto signal_raw = GetConfigValue<int>(
        configuration_->properties,
        WidgetProperty::GetTypeName(WidgetPropertyType::UI_SIGNAL_TYPE),
        0);

    auto signal = static_cast<UiSignalType>(signal_raw);
    if(signal == UiSignalType::None)
        return;

    SubscribeWhileActive(
        UiSignalChannel::GetInstance(),
        signal,
        [this](const UiSignalChannel::EventMessage& event) {
            if(auto it = event.payload.find(UiSignalPayloadType::Value); it != event.payload.end())
                if(const auto* is_active = std::get_if<bool>(&it->second))
                    SetIsActive(*is_active);
        });
}

} // namespace eerie_leap::views::widgets::basic
