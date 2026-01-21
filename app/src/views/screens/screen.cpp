#include <stdexcept>

#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "views/widgets/widget_factory.h"

#include "screen.h"

namespace eerie_leap::views::screens {

LOG_MODULE_REGISTER(screen_logger);

Screen::Screen(
    std::shared_ptr<UiAssetsManager> ui_assets_manager,
    uint32_t id,
    std::shared_ptr<Frame> parent)
        : ui_assets_manager_(std::move(ui_assets_manager)),
        id_(id),
        parent_(parent) {

    widgets_ = std::make_shared<std::vector<std::unique_ptr<IWidget>>>();

    container_ = std::make_shared<Frame>(Frame::CreateWrapped(parent->GetObject())
        .SetWidth(100, false)
        .SetHeight(100, false)
        .Build());
}

int Screen::DoRender() {
    for(auto& widget : *widgets_)
        widget->Render();

    return 0;
}

int Screen::ApplyTheme() {
    return 0;
}

void Screen::Configure(std::shared_ptr<ScreenConfiguration> configuration) {
    configuration_ = std::move(configuration);

    widgets_->clear();

    for(const auto& widget_config : configuration_->widget_configurations) {
        auto widget = WidgetFactory::GetInstance().CreateWidget(widget_config, container_);
        widget->SetAssetsManager(ui_assets_manager_);
        UpdateWidgetSize(*widget, configuration_->grid);
        UpdateWidgetPosition(*widget, configuration_->grid);

        widgets_->push_back(std::move(widget));
    }
}

std::shared_ptr<ScreenConfiguration> Screen::GetConfiguration() const {
    return configuration_;
}

std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> Screen::GetWidgets() const {
    return widgets_;
}

void Screen::UpdateWidgetSize(IWidget& widget, GridSettings& grid_settings) {
    lv_obj_t *active_screen = lv_screen_active();
    int32_t screen_width = lv_obj_get_width(active_screen);
    int32_t screen_height = lv_obj_get_height(active_screen);

    const auto& widget_config = widget.GetConfiguration();

    uint32_t width = 0;
    if(grid_settings.width == widget_config->size_grid.width)
        width = screen_width;
    else
        width = (screen_width / grid_settings.width) * widget_config->size_grid.width - grid_settings.spacing_px * 2;

    uint32_t height = 0;
    if(grid_settings.height == widget_config->size_grid.height)
        height = screen_height;
    else
        height = (screen_height / grid_settings.height) * widget_config->size_grid.height - grid_settings.spacing_px * 2;

    widget.SetSizePx({.width = width, .height = height});
}

void Screen::UpdateWidgetPosition(IWidget& widget, GridSettings& grid_settings) {
    lv_obj_t *active_screen = lv_screen_active();
    int32_t screen_width = lv_obj_get_width(active_screen);
    int32_t screen_height = lv_obj_get_height(active_screen);

    const auto& widget_config = widget.GetConfiguration();

    auto size_px = widget.GetSizePx();
    if(size_px.width == 0 || size_px.height == 0)
        throw std::runtime_error("Widget size is not set.");

    uint32_t cell_width = (screen_width / grid_settings.width) + (grid_settings.spacing_px * 2);
    uint32_t cell_height = (screen_height / grid_settings.height) + (grid_settings.spacing_px * 2);

    int x = cell_width * widget_config->position_grid.x;
    int y = screen_height - cell_height * widget_config->position_grid.y - size_px.height;
    y = std::abs(y);

    widget.SetPositionPx({.x = x, .y = y});
}

} // namespace eerie_leap::views::screens
