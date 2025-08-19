#include "domain/interface_domain/types/sensor_reading_dto.h"

#include "ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain::types;

UiController::UiController(std::shared_ptr<Interface> interface, std::shared_ptr<MainView> main_view) : interface_(std::move(interface)), main_view_(std::move(main_view)) {}

int UiController::Initialize() {
    main_view_->Render();

    return interface_->RegisterReadingHandler(2348664336, [this](SensorReadingDto& reading) {
        if(reading.status != ReadingStatus::PROCESSED)
            return -1;

        main_view_->Update(static_cast<int>(reading.value));

        return 0;
    });
}

} // namespace eerie_leap::controllers
