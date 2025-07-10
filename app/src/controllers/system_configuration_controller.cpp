#include "utilities/memory/heap_allocator.h"
#include "system_configuration_controller.h"

namespace eerie_leap::controllers {

LOG_MODULE_REGISTER(system_config_ctrl_logger);

SystemConfigurationController::SystemConfigurationController(std::shared_ptr<ConfigurationService<SystemConfig>> system_configuration_service) :
    system_configuration_service_(std::move(system_configuration_service)),
    system_config_(nullptr),
    system_configuration_(nullptr) {

    if(Get(true) == nullptr)
        LOG_ERR("Failed to load System configuration.");
    else
        LOG_INF("System Configuration Controller initialized successfully.");
}

bool SystemConfigurationController::Update(std::shared_ptr<SystemConfiguration> system_configuration) {
    SystemConfig system_config {
        .hw_version = system_configuration->hw_version,
        .sw_version = system_configuration->sw_version
    };

    if(!system_configuration_service_->Save(&system_config))
        return false;

    system_config_ = make_shared_ext<SystemConfig>(system_config);
    system_configuration_ = system_configuration;

    return true;
}

std::shared_ptr<SystemConfiguration> SystemConfigurationController::Get(bool force_load) {
    if (system_configuration_ != nullptr && !force_load) {
        return system_configuration_;
    }

    auto system_config = system_configuration_service_->Load();
    if(!system_config.has_value())
        return nullptr;

    system_config_raw_ = system_config.value().config_raw;
    system_config_ = system_config.value().config;

    SystemConfiguration system_configuration {
        .hw_version = system_config_->hw_version,
        .sw_version = system_config_->sw_version
    };
    system_configuration_ = make_shared_ext<SystemConfiguration>(system_configuration);

    return system_configuration_;
}

} // namespace eerie_leap::controllers
