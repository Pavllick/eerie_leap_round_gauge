#include "views/widgets/basic/label_widget/label_icon_widget.h"
#include "views/widgets/basic/arc_label_widget/arc_label_icon_widget.h"

#include "views/widgets/indicators/arc_fill_indicator/arc_fill_indicator.h"
#include "views/widgets/indicators/digital_indicator/digital_indicator.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

#include "widget_factory.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::views::widgets::basic;
using namespace eerie_leap::views::widgets::indicators;

WidgetFactory::WidgetFactory() {
    RegisterTypes();
}

WidgetFactory& WidgetFactory::GetInstance() {
    static WidgetFactory instance;
    return instance;
}

template<typename T>
void WidgetFactory::RegisterWidget(const WidgetType type) {
    creators_[type] = [](const uint32_t id, std::shared_ptr<Frame> parent) -> std::unique_ptr<IWidget> {
        return std::make_unique<T>(id, parent);
    };
}

void WidgetFactory::RegisterWidget(const WidgetType type, WidgetCreator creator) {
    creators_[type] = std::move(creator);
}

std::unique_ptr<IWidget> WidgetFactory::CreateWidget(const WidgetType type, const uint32_t id, std::shared_ptr<Frame> parent) {
    auto it = creators_.find(type);
    if (it == creators_.end())
        throw std::runtime_error("Unknown widget type");

    return it->second(id, parent);
}

std::unique_ptr<IWidget> WidgetFactory::CreateWidget(const WidgetConfiguration& config, std::shared_ptr<Frame> parent) {
    auto widget = CreateWidget(config.type, config.id, parent);
    widget->Configure(config);

    return widget;
}

std::vector<WidgetType> WidgetFactory::GetAvailableTypes() const {
    std::vector<WidgetType> types;
    types.reserve(creators_.size());

    for (const auto& [type, creator] : creators_)
        types.push_back(type);

    return types;
}

void WidgetFactory::RegisterTypes() {
    RegisterWidget<LabelIconWidget>(WidgetType::BasicLabelIcon);
    RegisterWidget<ArcLabelIconWidget>(WidgetType::BasicArcLabelIcon);

    RegisterWidget<ArcFillIndicator>(WidgetType::IndicatorArcFill);
    RegisterWidget<DigitalIndicator>(WidgetType::IndicatorDigital);
    RegisterWidget<HorizontalChartIndicator>(WidgetType::IndicatorHorizontalChart);
}

} // namespace eerie_leap::views::widgets
