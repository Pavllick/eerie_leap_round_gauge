#include <stdexcept>

#include <zephyr/logging/log.h>

#include "views/screens/screen.h"

#include "ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::views::screens;

LOG_MODULE_REGISTER(ui_controller_logger);

UiController::UiController(
    std::shared_ptr<UiConfigurationManager> ui_configuration_manager,
    std::shared_ptr<UiAssetsManager> ui_assets_manager)
    : ui_configuration_manager_(std::move(ui_configuration_manager)),
      ui_assets_manager_(std::move(ui_assets_manager)) {

    main_view_ = make_unique_ext<MainView>();
    Configure(ui_configuration_manager_->Get());
}

int UiController::Configure(std::shared_ptr<UiConfiguration> config) {
    configuration_ = std::move(config);

    for(auto& screen_config : configuration_->screen_configurations) {
        auto screen = CreateScreen(screen_config);

        if(screen != nullptr) {
            screens_.emplace(screen_config->id, screen);
            main_view_->AddScreen(screen_config->id, screen);
        }
    }

    main_view_->SetActiveScreen(configuration_->active_screen_index);

    return 0;
}

int UiController::Render() {
    main_view_->Render();

    return 0;
}

std::shared_ptr<IScreen> UiController::CreateScreen(std::shared_ptr<ScreenConfiguration> configuration) {
    try {
        auto screen = std::make_shared<Screen>(ui_assets_manager_, configuration->id, main_view_->GetContainer());
        screen->Configure(configuration);

        return screen;
    } catch(const std::exception& e) {
        LOG_ERR("Failed to create screen with ID: %d. %s", configuration->id, e.what());
        return nullptr;
    }
}

} // namespace eerie_leap::controllers
