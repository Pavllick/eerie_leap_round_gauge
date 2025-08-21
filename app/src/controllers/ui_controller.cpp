#include "domain/interface_domain/types/sensor_reading_dto.h"

#include "ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;

UiController::UiController(std::shared_ptr<ReadingProcessingService> reading_processing_service, std::shared_ptr<Interface> interface, std::shared_ptr<MainView> main_view)
    : reading_processing_service_(std::move(reading_processing_service)), interface_(std::move(interface)), main_view_(std::move(main_view)) {}

int UiController::Initialize(std::vector<uint32_t> sensor_ids) {
    main_view_->Render();

    for(auto sensor_id : sensor_ids) {
        current_display_values_[sensor_id] = 0;

        reading_processing_service_->RegisterReadingHandler(sensor_id, [this](SensorReadingDto& reading) {
            if(reading.status != ReadingStatus::PROCESSED)
                return -1;

            main_view_->Update(static_cast<int>(reading.value));

            return 0;
        });
    }

    return 0;
}

} // namespace eerie_leap::controllers
