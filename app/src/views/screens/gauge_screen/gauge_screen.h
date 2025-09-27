#pragma once

#include <memory>
#include <vector>

#include "domain/ui_domain/models/screen_configuration.h"
#include "domain/sensor_domain/models/sensor.h"

#include "views/widgets/i_widget.h"
#include "views/screens/i_screen.h"
#include "views/widgets/widget_factory.h"

namespace eerie_leap::views::screens {

using namespace eerie_leap::domain::sensor_domain::models;
using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets;

class GaugeScreen : public IScreen {
private:
    std::shared_ptr<std::vector<std::shared_ptr<IWidget>>> widgets_;
    ScreenConfiguration configuration_;
    std::shared_ptr<WidgetFactory> widget_factory_;
    std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors_;

public:
    explicit GaugeScreen(std::shared_ptr<WidgetFactory> widget_factory, std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors);

    int Render() override;
    void Configure(ScreenConfiguration& config) override;

    std::shared_ptr<std::vector<std::shared_ptr<IWidget>>> GetWidgets() const override;
};

} // namespace eerie_leap::views::screens
