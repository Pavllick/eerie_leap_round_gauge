#include "domain/interface_domain/types/sensor_reading_dto.h"

#include "gague_screen_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;
using namespace eerie_leap::views::screens;
using namespace eerie_leap::views::widgets::indicators;

GagueScreenController::GagueScreenController(std::shared_ptr<ReadingProcessorService> reading_processor_service)
    : reading_processor_service_(std::move(reading_processor_service)) { }

int GagueScreenController::InitializeScreen(std::shared_ptr<IScreen>& screen) {
    for(auto& widget : *screen->GetWidgets()) {
        if(auto indicator = dynamic_cast<IIndicator*>(widget.get()))
            RegisterIndicatorReadingHandler(indicator);
    }

    return 0;
}

void GagueScreenController::RegisterIndicatorReadingHandler(IIndicator* indicator) {
    auto sensor_id = indicator->GetSensorId();
        if(!sensor_id.has_value())
            return;

    reading_processor_service_->RegisterReadingHandler(sensor_id.value(), [indicator](SensorReadingDto& reading) {
        if(reading.status != ReadingStatus::PROCESSED)
            return -1;

        indicator->Update(reading.value);

        return 0;
    });
}

} // namespace eerie_leap::controllers
