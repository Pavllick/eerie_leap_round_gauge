#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "domain/ui_domain/configuration/ui_configuration_manager.h"
#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/screen_configuration.h"
#include "domain/ui_domain/models/widget_tag.h"

#include "controllers/gague_screen_controller.h"

#include "views/main_view.h"
#include "views/screens/i_screen.h"
#include "views/screens/screen.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::domain::ui_domain::configuration;
using namespace eerie_leap::domain::ui_domain::models;

using namespace eerie_leap::views;
using namespace eerie_leap::views::screens;
using namespace eerie_leap::views::widgets;

class UiController {
private:
    std::shared_ptr<UiConfigurationManager> ui_configuration_manager_;
    std::shared_ptr<GagueScreenController> gague_screen_controller_;

    std::shared_ptr<MainView> main_view_;
    UiConfiguration configuration_;

    std::unordered_map<uint32_t, std::shared_ptr<IScreen>> screens_;

    int Configure(UiConfiguration& config);

    std::shared_ptr<IScreen> CreateScreen(ScreenConfiguration& config);

public:
    UiController(std::shared_ptr<UiConfigurationManager> ui_configuration_manager,
        std::shared_ptr<GagueScreenController> gague_screen_controller);

    int UpdateWidgetPropertyByTag(const WidgetPropertyType property_type, const ConfigValue& value, WidgetTag tag, bool force_update = false);

    int Render();
};

} // namespace eerie_leap::controllers
