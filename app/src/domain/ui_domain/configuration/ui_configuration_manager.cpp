#include "utilities/cbor/cbor_helpers.hpp"
#include "utilities/memory/heap_allocator.h"

#include "ui_configuration_manager.h"

namespace eerie_leap::domain::ui_domain::configuration {

using namespace eerie_leap::utilities::cbor;
using namespace eerie_leap::utilities::memory;

LOG_MODULE_REGISTER(ui_config_ctrl_logger);

UiConfigurationManager::UiConfigurationManager(std::unique_ptr<CborConfigurationService<CborUiConfig>> cbor_configuration_service) :
    cbor_configuration_service_(std::move(cbor_configuration_service)),
    configuration_(nullptr) {

    cbor_parser_ = std::make_unique<UiConfigurationCborParser>();

    if(Get(true) == nullptr)
        LOG_ERR("Failed to load UI configuration.");
    else
        LOG_INF("UI Configuration Controller initialized successfully.");
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

    Get(true);

    return true;
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

} // namespace eerie_leap::domain::ui_domain::configuration
