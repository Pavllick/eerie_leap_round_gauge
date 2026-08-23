#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "subsys/assets/assets_manager.h"
#include "subsys/fs/services/i_fs_service.h"
#include "subsys/threading/work_queue_thread.h"

#include "domain/configuration_domain/services/configuration_service.h"
#include "domain/sensor_domain/utilities/sensor_readings_frame.hpp"

#include "domain/ui_domain/configuration/ui_configuration_manager.h"
#include "domain/ui_domain/event_bus/ui_event_bus.h"
#include "domain/ui_domain/models/ui_configuration.h"
#include "domain/ui_domain/models/screen_configuration.h"
#include "domain/ui_domain/services/ui_renderer_service.h"
#include "domain/ui_domain/services/sensors_rendering_service.h"
#include "domain/ui_domain/services/navigation_service.h"
#include "domain/ui_domain/services/settings_registry.h"
#include "domain/ui_domain/services/ui_input_service.h"

#include "views/main_view.h"
#include "views/screens/i_screen.h"
#include "views/widgets/widget_context.h"

namespace eerie_leap::controllers {

using eerie_leap::subsys::assets::AssetsManager;
using eerie_leap::subsys::fs::services::IFsService;
using eerie_leap::subsys::threading::WorkQueueThread;

using eerie_leap::domain::configuration_domain::services::ConfigurationService;
using eerie_leap::domain::sensor_domain::utilities::SensorReadingsFrame;

using eerie_leap::domain::ui_domain::models::UiConfiguration;
using eerie_leap::domain::ui_domain::models::ScreenConfiguration;
using eerie_leap::domain::ui_domain::configuration::UiConfigurationManager;
using eerie_leap::domain::ui_domain::event_bus::UiEvent;
using eerie_leap::domain::ui_domain::event_bus::UiSubscriptionHandle;
using eerie_leap::domain::ui_domain::services::UiRendererService;
using eerie_leap::domain::ui_domain::services::SensorsRenderingService;
using eerie_leap::domain::ui_domain::services::NavigationService;
using eerie_leap::domain::ui_domain::services::SettingsRegistry;
using eerie_leap::domain::ui_domain::services::UiInputService;

using eerie_leap::views::MainView;
using eerie_leap::views::screens::IScreen;
using eerie_leap::views::widgets::WidgetContext;

class UiController {
private:
    static constexpr const char* UI_CONFIGURATION_NAME = "ui_config";
    static constexpr const char* UI_ASSETS_DIR = "ui_assets";

    std::shared_ptr<IFsService> fs_service_;
    std::shared_ptr<WorkQueueThread> config_work_queue_thread_;
    std::shared_ptr<ConfigurationService> configuration_service_;
    std::shared_ptr<SensorReadingsFrame> sensor_readings_frame_;

    std::shared_ptr<UiConfigurationManager> ui_configuration_manager_;
    std::shared_ptr<AssetsManager> ui_assets_manager_;
    std::shared_ptr<UiRendererService> ui_renderer_service_;
    std::shared_ptr<SensorsRenderingService> sensors_rendering_service_;
    std::shared_ptr<NavigationService> navigation_service_;
    std::shared_ptr<SettingsRegistry> settings_registry_;

    // Declared before ui_input_service_, which detaches from the view's LVGL object on teardown.
    std::unique_ptr<MainView> main_view_;
    std::shared_ptr<UiInputService> ui_input_service_;

    WidgetContext widget_context_;
    std::shared_ptr<UiConfiguration> configuration_;

    std::optional<UiSubscriptionHandle> navigation_subscription_;

    int Configure(std::shared_ptr<UiConfiguration> config);

    std::shared_ptr<IScreen> CreateScreen(std::shared_ptr<ScreenConfiguration> configuration);

    void SubscribeToNavigation();
    void OnNavigationChanged(const UiEvent& event);

    // TODO: For test purposes only
    void SetupTestConfiguration();
    void SetupTestAssets();

public:
    UiController(
        std::shared_ptr<IFsService> fs_service,
        std::shared_ptr<WorkQueueThread> config_work_queue_thread,
        std::shared_ptr<ConfigurationService> configuration_service,
        std::shared_ptr<SensorReadingsFrame> sensor_readings_frame);
    ~UiController();

    int Initialize();
    int Start();

    std::shared_ptr<NavigationService> GetNavigationService() const;
    std::shared_ptr<SettingsRegistry> GetSettingsRegistry() const;
};

} // namespace eerie_leap::controllers
