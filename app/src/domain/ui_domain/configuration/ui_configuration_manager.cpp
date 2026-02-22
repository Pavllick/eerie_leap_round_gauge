#include "utilities/cbor/cbor_helpers.hpp"
#include "utilities/memory/heap_allocator.h"

#include "ui_configuration_manager.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::utilities::cbor;
using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(ui_config_ctrl_logger);

UiConfigurationManager::UiConfigurationManager(
    std::unique_ptr<CborConfigurationService<CborUiConfig>> cbor_configuration_service,
    std::unique_ptr<JsonConfigurationService<JsonUiConfig>> json_configuration_service) :
    cbor_configuration_service_(std::move(cbor_configuration_service)),
    json_configuration_service_(std::move(json_configuration_service)),
    configuration_(nullptr) {

    cbor_parser_ = std::make_unique<UiConfigurationCborParser>();
    json_parser_ = std::make_unique<UiConfigurationJsonParser>();
    std::shared_ptr<UiConfiguration> configuration = nullptr;

    try {
        configuration = Get(true);
    } catch(...) {
        LOG_ERR("Failed to load UI configuration.");
    }

    if(configuration == nullptr) {
        if(!CreateDefaultConfiguration()) {
            LOG_ERR("Failed to create default UI configuration.");
            return;
        }

        LOG_INF("Default UI configuration loaded successfully.");
    }

    LOG_INF("UI Configuration Manager initialized successfully.");

    ApplyJsonConfiguration(true);
}

bool UiConfigurationManager::ApplyJsonConfiguration(bool fs_load, std::span<const uint8_t> data) {
    if(fs_load && !json_configuration_service_->IsAvailable())
        return false;

    auto json_config_loaded = fs_load
        ? json_configuration_service_->Load()
        : json_configuration_service_->Load(data);
    if(!json_config_loaded.has_value())
        return false;

    // if(json_config_loaded->checksum == json_config_checksum_)
    //     return true;

    try {
        auto configuration = json_parser_->Deserialize(Mrm::GetExtPmr(), *json_config_loaded->config);

        if(!Update(*configuration))
            return false;
    } catch(const std::exception& e) {
        LOG_ERR("Failed to deserialize JSON configuration. %s", e.what());
        return false;
    }

    LOG_INF("JSON configuration loaded successfully.");

    return true;
}

bool UiConfigurationManager::ApplyJsonConfiguration(std::span<const uint8_t> data) {
    return ApplyJsonConfiguration(false, data);
}

bool UiConfigurationManager::Update(const UiConfiguration& configuration) {
    try {
        auto cbor_config = cbor_parser_->Serialize(configuration);

        if(!cbor_configuration_service_->Save(cbor_config.get()))
            return false;
    } catch(const std::exception& e) {
        LOG_ERR("Failed to update UI configuration. %s", e.what());
        return false;
    }

    return Get(true) != nullptr;
}

std::shared_ptr<UiConfiguration> UiConfigurationManager::Get(bool force_load) {
    if(configuration_ != nullptr && !force_load)
        return configuration_;

    auto cbor_config_data = cbor_configuration_service_->Load();
    if(!cbor_config_data.has_value())
        return nullptr;

    auto cbor_config = std::move(cbor_config_data.value().config);

    auto configuration = cbor_parser_->Deserialize(Mrm::GetExtPmr(), *cbor_config);
    configuration_ = std::make_shared<UiConfiguration>(std::move(*configuration));

    return configuration_;
}

bool UiConfigurationManager::CreateDefaultConfiguration() {
    auto configuration = make_unique_pmr<UiConfiguration>(Mrm::GetExtPmr());

    return Update(*configuration);
}

} // namespace eerie_leap::domain::ui_domain::configuration
