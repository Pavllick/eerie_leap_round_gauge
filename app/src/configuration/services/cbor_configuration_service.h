#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <optional>
#include <span>
#include <zephyr/logging/log.h>

#include "utilities/memory/memory_resource_manager.h"
#include "configuration/cbor/traits/system_config_trait.h"
#include "configuration/cbor/traits/ui_config_trait.h"
#include "subsys/fs/services/i_fs_service.h"
#include "utilities/cbor/cbor_serializer.h"
#include "loaded_config.hpp"

namespace eerie_leap::configuration::services {

using namespace eerie_leap::utilities::memory;
using namespace eerie_leap::configuration::cbor::traits;
using namespace eerie_leap::subsys::fs::services;
using namespace eerie_leap::utilities::cbor;

template <typename T>
class CborConfigurationService {
private:
    const std::string configuration_dir_ = "config";
    static constexpr size_t load_buffer_size_ = sizeof(T) + 2048;

    std::string configuration_name_;
    std::shared_ptr<IFsService> fs_service_;
    std::shared_ptr<CborSerializer<T>> cbor_serializer_;

    const std::string configuration_file_path_ = configuration_dir_ + "/" + configuration_name_ + ".cbor";

public:
    CborConfigurationService(std::string configuration_name, std::shared_ptr<IFsService> fs_service)
        : configuration_name_(std::move(configuration_name)), fs_service_(std::move(fs_service)) {

        cbor_serializer_ = make_shared_ext<CborSerializer<T>>(
            CborTrait<T>::Encode,
            CborTrait<T>::Decode,
            CborTrait<T>::GetSerializingSize);

        if (!fs_service_->Exists(configuration_dir_))
            fs_service_->CreateDirectory(configuration_dir_);
    }

    bool Save(T* configuration) {
        LOG_MODULE_DECLARE(configuration_service_logger);

        auto config_bytes = cbor_serializer_->Serialize(*configuration);
        LOG_INF("Saved %s configuration size: %d", configuration_file_path_.c_str(), config_bytes.size());

        if (config_bytes.empty()) {
            LOG_ERR("Failed to serialize configuration %s.", configuration_file_path_.c_str());
            return false;
        }

        return fs_service_->WriteFile(configuration_file_path_, config_bytes.data(), config_bytes.size());
    }

    std::optional<LoadedConfig<T>> Load() {
        LOG_MODULE_DECLARE(configuration_service_logger);

        if (!fs_service_->Exists(configuration_file_path_)) {
            LOG_ERR("Configuration file %s does not exist.", configuration_file_path_.c_str());
            return std::nullopt;
        }

        size_t buffer_size = fs_service_->GetFileSize(configuration_file_path_);
        std::pmr::vector<uint8_t> buffer(buffer_size, Mrm::GetExtPmr());
        size_t out_len = 0;

        if (!fs_service_->ReadFile(configuration_file_path_, buffer.data(), buffer_size, out_len)) {
            LOG_ERR("Failed to read configuration file %s.", configuration_file_path_.c_str());
            return std::nullopt;
        }

        auto config_bytes = std::span<const uint8_t>(buffer.data(), out_len);
        auto configuration = cbor_serializer_->Deserialize(config_bytes);

        if (configuration == nullptr) {
            LOG_ERR("Failed to deserialize configuration %s.", configuration_file_path_.c_str());
            return std::nullopt;
        }

        LoadedConfig<T> loaded_config {
            .config_raw = std::move(buffer),
            .config = std::move(configuration)
        };

        LOG_INF("Loaded %s configuration successfully. Size: %d", configuration_file_path_.c_str(), out_len);

        return loaded_config;
    }
};

} // namespace eerie_leap::configuration::services
