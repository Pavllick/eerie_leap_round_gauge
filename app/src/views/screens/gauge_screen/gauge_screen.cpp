#include <stdexcept>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

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

void UpdateWidgetSize(std::unique_ptr<IWidget>& widget, GridSettings& grid_settings) {
    lv_obj_t *active_screen = lv_screen_active();
    int32_t screen_width = lv_obj_get_width(active_screen);
    int32_t screen_height = lv_obj_get_height(active_screen);

    auto widget_config = widget->GetConfiguration();

    uint32_t width = (screen_width / grid_settings.width) * widget_config.size_grid.width - grid_settings.spacing_px * 2;
    uint32_t height = (screen_height / grid_settings.height) * widget_config.size_grid.height - grid_settings.spacing_px * 2;

    widget->SetSizePx({.width = width, .height = height});
}

void UpdateWidgetPosition(std::unique_ptr<IWidget>& widget, GridSettings& grid_settings) {
    lv_obj_t *active_screen = lv_screen_active();
    int32_t screen_width = lv_obj_get_width(active_screen);
    int32_t screen_height = lv_obj_get_height(active_screen);

    auto widget_config = widget->GetConfiguration();

    auto size_px = widget->GetSizePx();
    if(size_px.width == 0 || size_px.height == 0)
        throw std::runtime_error("Widget size is not set.");

    uint32_t cell_width = (screen_width / grid_settings.width) + (grid_settings.spacing_px * 2);
    uint32_t cell_height = (screen_height / grid_settings.height) + (grid_settings.spacing_px * 2);

    int x = cell_width * widget_config.position_grid.x;
    int y = screen_height - cell_height * widget_config.position_grid.y - size_px.height;
    y = std::abs(y);

    widget->SetPositionPx({.x = x, .y = y});
}

void GaugeScreen::Configure(ScreenConfiguration& config) {
    configuration_ = config;

    widgets_->clear();

    for(auto& widget_config : configuration_.widget_configurations) {
        auto widget = widget_factory_->CreateWidget(widget_config);
        UpdateWidgetSize(widget, configuration_.grid);
        UpdateWidgetPosition(widget, configuration_.grid);

        widgets_->push_back(std::move(widget));
    }
}

std::shared_ptr<std::vector<std::shared_ptr<IWidget>>> GaugeScreen::GetWidgets() const {
    return widgets_;
}

} // namespace eerie_leap::views::screens
