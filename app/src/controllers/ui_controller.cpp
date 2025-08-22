#include "domain/interface_domain/types/sensor_reading_dto.h"

#include "ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;

UiController::UiController(std::shared_ptr<ReadingProcessorService> reading_processor_service, std::shared_ptr<Interface> interface, std::shared_ptr<MainView> main_view)
    : reading_processor_service_(std::move(reading_processor_service)), interface_(std::move(interface)), main_view_(std::move(main_view)) {}

int UiController::Initialize(std::vector<uint32_t> sensor_ids) {
    main_view_->Render();

    for(auto sensor_id : sensor_ids) {
        current_display_values_[sensor_id] = 0;

        reading_processor_service_->RegisterReadingHandler(sensor_id, [this](SensorReadingDto& reading) {
            if(reading.status != ReadingStatus::PROCESSED)
                return -1;

            main_view_->Update(reading.value);

            return 0;
        });
    }

    return 0;
}

} // namespace eerie_leap::controllers
