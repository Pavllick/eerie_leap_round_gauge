#include "views/widgets/indicators/arc_fill_indicator/arc_fill_indicator.h"
#include "views/widgets/indicators/digital_indicator/digital_indicator.h"
#include "views/widgets/indicators/horizontal_chart_indicator/horizontal_chart_indicator.h"

#include "widget_factory.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::views::widgets::indicators;

std::shared_ptr<WidgetFactory> WidgetFactory::instance_ = nullptr;

WidgetFactory::WidgetFactory() {
    RegisterBuiltinWidgets();
}

std::shared_ptr<WidgetFactory> WidgetFactory::GetInstance() {
    if (!instance_) {
        WidgetFactory factory;
        instance_ = std::make_shared<WidgetFactory>(factory);
    }

    return instance_;
}

void WidgetFactory::RegisterWidget(const WidgetType type, WidgetCreator creator) {
    creators_[type] = std::move(creator);
}

template<typename T>
void WidgetFactory::RegisterWidget(const WidgetType type) {
    creators_[type] = [](const uint32_t id) -> std::unique_ptr<IWidget> {
        return std::make_unique<T>(id);
    };
}

std::unique_ptr<IWidget> WidgetFactory::CreateWidget(const WidgetType type, const uint32_t id) {
    auto it = creators_.find(type);
    if (it == creators_.end())
        throw std::runtime_error("Unknown widget type");

    return it->second(id);
}

std::unique_ptr<IWidget> WidgetFactory::CreateWidget(const WidgetConfiguration& config) {
    auto widget = CreateWidget(config.type, config.id);
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

bool WidgetFactory::IsTypeRegistered(const WidgetType type) const {
    return creators_.contains(type);
}

void WidgetFactory::RegisterBuiltinWidgets() {
    RegisterWidget<ArcFillIndicator>(WidgetType::IndicatorArcFill);
    RegisterWidget<DigitalIndicator>(WidgetType::IndicatorDigital);
    RegisterWidget<HorizontalChartIndicator>(WidgetType::IndicatorHorizontalChart);
}

} // namespace eerie_leap::views::widgets
