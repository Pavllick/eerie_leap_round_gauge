#include "utilities/memory/heap_allocator.h"
#include "domain/interface_domain/types/sensor_reading_dto.h"
#include "views/screens/gauge_screen/gauge_screen.h"
#include "views/widgets/indicators/i_indicator.h"

#include "gague_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::domain::interface_domain::types;
using namespace eerie_leap::views::screens;
using namespace eerie_leap::views::widgets::indicators;

GagueController::GagueController(
    std::shared_ptr<GaugeConfigurationController> gauge_configuration_controller,
    std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors,
    std::shared_ptr<ReadingProcessorService> reading_processor_service,
    std::shared_ptr<WidgetFactory> widget_factory)
    : gauge_configuration_controller_(std::move(gauge_configuration_controller)),
    sensors_(std::move(sensors)),
    reading_processor_service_(std::move(reading_processor_service)),
    widget_factory_(std::move(widget_factory)) {

        main_view_ = make_shared_ext<MainView>();
        Configure(*gauge_configuration_controller_->Get());
    }

int GagueController::Configure(GaugeConfiguration& config) {
    configuration_ = config;

    for(auto& screen_config : config.screen_configurations) {
        auto screen = CreateScreen(screen_config, sensors_);

        for(auto& widget : *screen->GetWidgets())
            RegisterIndicatorReadingHandler(widget);

        screens_.insert({ screen_config.id, screen });
        main_view_->AddScreen(screen_config.id, std::move(screen));
    }

    main_view_->SetActiveScreen(config.active_screen_index);

    return 0;
}

int GagueController::RegisterIndicatorReadingHandler(std::shared_ptr<IWidget> widget) {
    if(auto indicator = dynamic_cast<IIndicator*>(widget.get())) {
        auto sensor_id = indicator->GetSensorId();
        if(!sensor_id.has_value())
            return -1;

        auto sensor = GetSensor(sensor_id.value());
        if(sensor == nullptr)
            return -1;

        reading_processor_service_->RegisterReadingHandler(sensor->id_hash, [indicator](SensorReadingDto& reading) {
            if(reading.status != ReadingStatus::PROCESSED)
                return -1;

            indicator->Update(reading.value);

            return 0;
        });
    }

    return 0;
}

int GagueController::Render() {
    main_view_->Render();

    return 0;
}

int GagueController::UpdateWidgetProperty(const WidgetPropertyType property_type, const ConfigValue& value, WidgetTag tag, bool force_update) {
    for(auto& [id, screen] : screens_) {
        for(auto& widget : *screen->GetWidgets()) {
            if(widget->HasTag(tag))
                widget->UpdateProperty(property_type, value, force_update);
        }
    }

    return 0;
}

std::shared_ptr<IScreen> GagueController::CreateScreen(ScreenConfiguration& config, std::shared_ptr<std::vector<std::shared_ptr<Sensor>>> sensors) {
    auto screen = std::make_unique<GaugeScreen>(widget_factory_, sensors);
    screen->Configure(config);

    return screen;
}

std::shared_ptr<Sensor> GagueController::GetSensor(uint32_t id) {
    for(auto& sensor : *sensors_) {
        if(sensor->id_hash == id)
            return sensor;
    }

    return nullptr;
}

} // namespace eerie_leap::controllers
