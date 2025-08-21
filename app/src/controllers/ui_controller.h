#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "domain/interface_domain/interface.h"
#include "domain/sensor_domain/services/reading_processing_service.h"
#include "views/main/main_view.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::sensor_domain::services;
using namespace eerie_leap::views::main;

class UiController {
private:
    std::shared_ptr<ReadingProcessingService> reading_processing_service_;
    std::shared_ptr<Interface> interface_;
    std::shared_ptr<MainView> main_view_;

    std::unordered_map<uint32_t, float> current_display_values_;

public:
    UiController(std::shared_ptr<ReadingProcessingService> reading_processing_service, std::shared_ptr<Interface> interface, std::shared_ptr<MainView> main_view);

    int Initialize(std::vector<uint32_t> sensor_ids);
};

} // namespace eerie_leap::controllers
