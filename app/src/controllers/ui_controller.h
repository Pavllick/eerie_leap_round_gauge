#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "subsys/assets/assets_manager.h"
#include "domain/ui_domain/configuration/ui_configuration_manager.h"
#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/screen_configuration.h"

#include "views/main_view.h"
#include "views/screens/i_screen.h"

namespace eerie_leap::controllers {

using eerie_leap::subsys::assets::AssetsManager;
using eerie_leap::domain::ui_domain::models::UiConfiguration;
using eerie_leap::domain::ui_domain::models::ScreenConfiguration;
using eerie_leap::domain::ui_domain::configuration::UiConfigurationManager;

using eerie_leap::views::MainView;
using eerie_leap::views::screens::IScreen;

class UiController {
private:
    std::shared_ptr<UiConfigurationManager> ui_configuration_manager_;
    std::shared_ptr<AssetsManager> ui_assets_manager_;

    std::unique_ptr<MainView> main_view_;
    std::shared_ptr<UiConfiguration> configuration_;

    std::unordered_map<uint32_t, std::shared_ptr<IScreen>> screens_;

    int Configure(std::shared_ptr<UiConfiguration> config);

    std::shared_ptr<IScreen> CreateScreen(std::shared_ptr<ScreenConfiguration> configuration);

public:
    UiController(
        std::shared_ptr<UiConfigurationManager> ui_configuration_manager,
        std::shared_ptr<AssetsManager> ui_assets_manager);

    int Render();
};

} // namespace eerie_leap::controllers
