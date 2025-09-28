#include "utilities/memory/heap_allocator.h"

#include "ui_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::views::screens;

UiController::UiController(std::shared_ptr<UiConfigurationManager> ui_configuration_manager,
    std::shared_ptr<GagueScreenController> gague_screen_controller)
    : ui_configuration_manager_(std::move(ui_configuration_manager)),
    gague_screen_controller_(std::move(gague_screen_controller)) {

    main_view_ = make_shared_ext<MainView>();
    Configure(*ui_configuration_manager_->Get());
}

int UiController::Configure(UiConfiguration& config) {
    configuration_ = config;

    for(auto& screen_config : config.screen_configurations) {
        auto screen = CreateScreen(screen_config);

        if(screen_config.type == ScreenType::Gauge)
            gague_screen_controller_->InitializeScreen(screen);

        screens_.insert({ screen_config.id, screen });
        main_view_->AddScreen(screen_config.id, std::move(screen));
    }

    main_view_->SetActiveScreen(config.active_screen_index);

    return 0;
}

int UiController::Render() {
    main_view_->Render();

    return 0;
}

int UiController::UpdateWidgetPropertyByTag(const WidgetPropertyType property_type, const ConfigValue& value, WidgetTag tag, bool force_update) {
    for(auto& [id, screen] : screens_) {
        for(auto& widget : *screen->GetWidgets()) {
            if(widget->HasTag(tag))
                widget->UpdateProperty(property_type, value, force_update);
        }
    }

    return 0;
}

std::shared_ptr<IScreen> UiController::CreateScreen(ScreenConfiguration& config) {
    auto screen = std::make_unique<Screen>(config.id);
    screen->Configure(config);

    return screen;
}

} // namespace eerie_leap::controllers
