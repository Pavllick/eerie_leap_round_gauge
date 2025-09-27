#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "domain/sensor_domain/models/sensor.h"
#include "domain/sensor_domain/services/reading_processor_service.h"
#include "domain/ui_domain/configuration/ui_configuration_manager.h"

#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/screen_configuration.h"

#include "views/main_view.h"
#include "views/widgets/widget_factory.h"
#include "domain/ui_domain/models/widget_tag.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::sensor_domain::models;
using namespace eerie_leap::domain::sensor_domain::services;

using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::models;

using namespace eerie_leap::views;
using namespace eerie_leap::views::widgets;

class GagueController {
private:
    std::shared_ptr<UiConfigurationManager> ui_configuration_manager_;
    std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors_;
    std::shared_ptr<ReadingProcessorService> reading_processor_service_;
    std::shared_ptr<WidgetFactory> widget_factory_;
    std::shared_ptr<MainView> main_view_;
    UiConfiguration configuration_;

    std::unordered_map<uint32_t, std::shared_ptr<IScreen>> screens_;

    int Configure(UiConfiguration& config);

    int RegisterIndicatorReadingHandler(std::shared_ptr<IWidget> widget);
    std::shared_ptr<IScreen> CreateScreen(ScreenConfiguration& config, std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors);
    std::shared_ptr<Sensor> GetSensor(uint32_t id);

public:
    GagueController(
        std::shared_ptr<UiConfigurationManager> ui_configuration_manager,
        std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors,
        std::shared_ptr<ReadingProcessorService> reading_processor_service,
        std::shared_ptr<WidgetFactory> widget_factory);

    int UpdateWidgetProperty(const WidgetPropertyType property_type, const ConfigValue& value, WidgetTag tag, bool force_update = false);

    int Render();
};

} // namespace eerie_leap::controllers
