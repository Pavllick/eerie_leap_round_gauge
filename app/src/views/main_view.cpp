#include <string>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "main_view.h"

namespace eerie_leap::views {

LOG_MODULE_REGISTER(main_view_logger);

MainView::MainView() : radial_digital_gauge_(0, 100) {}

int MainView::Render() {
    radial_digital_gauge_.Render();

    return 0;
}

void MainView::Update(float value) {
    radial_digital_gauge_.Update(value);
}

} // namespace eerie_leap::views
