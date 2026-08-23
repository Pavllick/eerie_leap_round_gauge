#include <zephyr/logging/log.h>

#include "utilities/memory/memory_resource_manager.h"

#include "display_configuration_manager.h"

namespace eerie_leap::domain::display_domain::configuration {

using namespace eerie_memory;
using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::configuration::services;

LOG_MODULE_REGISTER(display_config_ctrl_logger);

DisplayConfigurationManager::DisplayConfigurationManager(
    std::unique_ptr<CborConfigurationService<CborDisplayConfig>> cbor_configuration_service) :
    cbor_configuration_service_(std::move(cbor_configuration_service)),
    configuration_(nullptr) {

    cbor_parser_ = std::make_unique<DisplayConfigurationCborParser>();
    std::shared_ptr<DisplayConfiguration> configuration = nullptr;

    try {
        configuration = Get(true);
    } catch(const std::exception& e) {
        LOG_ERR("Failed to load Display configuration. %s", e.what());
    } catch(...) {
        LOG_ERR("Failed to load Display configuration.");
    }

    if(configuration == nullptr) {
        if(!CreateDefaultConfiguration()) {
            LOG_ERR("Failed to create default Display configuration.");
            return;
        }

        LOG_INF("Default Display configuration loaded successfully.");
    }

    LOG_INF("Display Configuration Manager initialized successfully.");
}

void DisplayConfigurationManager::RegisterConfigurationUpdatedHandler(ConfigurationUpdatedHandler handler) {
    configuration_updated_handler_ = std::move(handler);
}

bool DisplayConfigurationManager::ApplyCborConfiguration(std::span<const uint8_t> cbor_data) {
    auto cbor_config = cbor_configuration_service_->Deserialize(cbor_data);
    if(cbor_config == nullptr)
        return false;

    try {
        auto configuration = cbor_parser_->Deserialize(Mrm::GetExtPmr(), *cbor_config);

        if(!Update(*configuration))
            return false;
    } catch(const std::exception& e) {
        LOG_ERR("Failed to deserialize CBOR configuration. %s", e.what());
        return false;
    }

    // Only the externally supplied configuration needs to be pushed to the
    // driver; a local Update() comes from the service that already applied it.
    if(configuration_updated_handler_) {
        try {
            configuration_updated_handler_();
        } catch(const std::exception& e) {
            LOG_ERR("Display configuration updated handler failed. %s", e.what());
        } catch(...) {
            LOG_ERR("Display configuration updated handler failed.");
        }
    }

    LOG_INF("CBOR configuration loaded successfully.");

    return true;
}

std::pmr::vector<uint8_t> DisplayConfigurationManager::GetCborConfiguration() {
    auto configuration = Get();

    auto cbor_config = cbor_parser_->Serialize(*configuration);

    return cbor_configuration_service_->Serialize(*cbor_config);
}

bool DisplayConfigurationManager::Update(const DisplayConfiguration& configuration) {
    try {
        auto cbor_config = cbor_parser_->Serialize(configuration);

        if(!cbor_configuration_service_->Save(cbor_config.get()))
            return false;
    } catch(const std::exception& e) {
        LOG_ERR("Failed to update Display configuration. %s", e.what());
        return false;
    }

    return Get(true) != nullptr;
}

std::shared_ptr<DisplayConfiguration> DisplayConfigurationManager::Get(bool force_load) {
    if(configuration_ != nullptr && !force_load)
        return configuration_;

    auto cbor_config_data = cbor_configuration_service_->Load();
    if(!cbor_config_data.has_value())
        return nullptr;

    auto cbor_config = std::move(cbor_config_data.value().config);

    auto configuration = cbor_parser_->Deserialize(Mrm::GetExtPmr(), *cbor_config);
    configuration_ = std::make_shared<DisplayConfiguration>(std::move(*configuration));

    return configuration_;
}

bool DisplayConfigurationManager::CreateDefaultConfiguration() {
    auto configuration = make_unique_pmr<DisplayConfiguration>(Mrm::GetExtPmr());

    return Update(*configuration);
}

} // namespace eerie_leap::domain::display_domain::configuration
