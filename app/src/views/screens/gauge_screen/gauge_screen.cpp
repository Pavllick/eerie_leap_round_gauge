#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "gauge_screen.h"

namespace eerie_leap::views::screens {

LOG_MODULE_REGISTER(gauge_screen_logger);

GaugeScreen::GaugeScreen(std::shared_ptr<WidgetFactory> widget_factory, std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors)
    : widget_factory_(std::move(widget_factory)), sensors_(std::move(sensors)) {
        widgets_ = std::make_shared<std::vector<std::shared_ptr<IWidget>>>();
    }

int GaugeScreen::Render() {
    for(auto& widget : *widgets_)
        widget->Render();

    return 0;
}

void GaugeScreen::Configure(ScreenConfiguration& config) {
    configuration_ = config;

    widgets_->clear();

    for(auto& widget_config : configuration_.widget_configurations) {
        auto widget = widget_factory_->CreateWidget(widget_config);
        widgets_->push_back(std::move(widget));
    }
}

std::shared_ptr<std::vector<std::shared_ptr<IWidget>>> GaugeScreen::GetWidgets() const {
    return widgets_;
}

} // namespace eerie_leap::views::screens
