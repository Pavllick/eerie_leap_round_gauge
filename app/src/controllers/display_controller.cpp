#include <cerrno>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include <zephyr/logging/log.h>
#include <eerie_memory.hpp>

#include "utilities/memory/memory_resource_manager.h"

#include "configuration/cbor/cbor_display_config/cbor_display_config.h"
#include "configuration/services/cbor_configuration_service.h"

#include "domain/settings_domain/models/setting_id.h"

#include "display_controller.h"

namespace eerie_leap::controllers {

using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;

namespace config_services = eerie_leap::configuration::services;

using eerie_leap::domain::settings_domain::models::SettingId;
using eerie_leap::domain::settings_domain::utilities::ToSettingNumber;
using eerie_leap::utilities::type::ConfigValue;

LOG_MODULE_REGISTER(display_controller_logger);

DisplayController::DisplayController(
    std::shared_ptr<IFsService> fs_service,
    std::shared_ptr<WorkQueueThread> config_work_queue_thread,
    std::shared_ptr<ConfigurationService> configuration_service,
    std::shared_ptr<SettingsRegistry> settings_registry)
        : fs_service_(std::move(fs_service)),
        config_work_queue_thread_(std::move(config_work_queue_thread)),
        configuration_service_(std::move(configuration_service)),
        settings_registry_(std::move(settings_registry)) {}

int DisplayController::Initialize() {
    auto cbor_display_config_service = std::make_unique<config_services::CborConfigurationService<CborDisplayConfig>>(
        DISPLAY_CONFIGURATION_NAME, fs_service_, config_work_queue_thread_);
    display_configuration_manager_ = std::make_shared<DisplayConfigurationManager>(
        std::move(cbor_display_config_service));

    if(configuration_service_ != nullptr)
        configuration_service_->RegisterCborConfigurationManager(
            ConfigurationService::Type::Display, display_configuration_manager_);

    display_service_ = std::make_shared<DisplayService>(
        display_configuration_manager_, config_work_queue_thread_);

    if(display_service_->Initialize() != 0) {
        LOG_ERR("Failed to initialize the display service.");
        return 0;
    }

    // Weak, because the service owns the manager that owns this handler.
    std::weak_ptr<DisplayService> display_service = display_service_;

    // A configuration pushed over BLE has to reach the panel without a reboot.
    display_configuration_manager_->RegisterConfigurationUpdatedHandler([display_service] {
        if(auto service = display_service.lock())
            service->Reload();
    });

    RegisterDisplaySettings();

    return 0;
}

void DisplayController::RegisterDisplaySettings() {
    if(display_service_ == nullptr || !display_service_->IsInitialized())
        return;

    // Captures the service, never `this`: bindings outlive individual widgets
    // and are invoked from the LVGL renderer thread.
    auto display_service = display_service_;

    int res = settings_registry_->Register(SettingId::DISPLAY_BRIGHTNESS, {
        .get = [display_service] {
            return ConfigValue { static_cast<int>(display_service->GetBrightness()) };
        },
        .set = [display_service](const ConfigValue& value) -> int {
            auto number = ToSettingNumber(value);
            if(!number.has_value())
                return -EINVAL;

            auto clamped = std::clamp(*number, display_brightness_min_, display_brightness_max_);

            return display_service->SetBrightness(static_cast<uint8_t>(std::lround(clamped)));
        },
        .commit = [display_service] { return display_service->Persist(); },
        .range = {
            .min = display_brightness_min_,
            .max = display_brightness_max_,
            .step = display_brightness_step_
        }
    });

    if(res != 0)
        LOG_ERR("Failed to register the '%s' setting. Error: %d.", SettingId::DISPLAY_BRIGHTNESS, res);
}

std::shared_ptr<DisplayService> DisplayController::GetDisplayService() const {
    return display_service_;
}

} // namespace eerie_leap::controllers
