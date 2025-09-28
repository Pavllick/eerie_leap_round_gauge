#pragma once

#include <memory>

#include "domain/sensor_domain/services/reading_processor_service.h"

#include "views/screens/i_screen.h"
#include "views/widgets/indicators/i_indicator.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::interface_domain;
using namespace eerie_leap::domain::sensor_domain::services;

using namespace eerie_leap::views;
using namespace eerie_leap::views::screens;
using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::widgets::indicators;

class GagueScreenController {
private:
    std::shared_ptr<ReadingProcessorService> reading_processor_service_;

    void RegisterIndicatorReadingHandler(IIndicator* indicator);

public:
    GagueScreenController(std::shared_ptr<ReadingProcessorService> reading_processor_service);

    int InitializeScreen(std::shared_ptr<IScreen>& screen);
};

} // namespace eerie_leap::controllers
