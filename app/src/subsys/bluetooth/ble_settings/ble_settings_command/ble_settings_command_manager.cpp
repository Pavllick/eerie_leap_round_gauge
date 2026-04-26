#include "ble_settings_command_manager.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandManager::BleSettingsCommandManager(std::shared_ptr<BleSettingsStatus> status)
    : status_(std::move(status)) {

    commands_.emplace(BleSettingsCommandType::StartWrite,
        std::make_unique<BleSettingsCommandStartWrite>(status_));
    commands_.emplace(BleSettingsCommandType::EndWrite,
        std::make_unique<BleSettingsCommandEndWrite>(status_));
    commands_.emplace(BleSettingsCommandType::RequestRead,
        std::make_unique<BleSettingsCommandRequestRead>(status_));
    commands_.emplace(BleSettingsCommandType::Abort,
        std::make_unique<BleSettingsCommandAbort>(status_));
}

void BleSettingsCommandManager::Initialize(
    std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer,
    Callbacks callbacks) {

    transfer_buffer_ = std::move(transfer_buffer);
    callbacks_ = std::move(callbacks);

    auto* end_write_cmd = static_cast<BleSettingsCommandEndWrite*>(
        commands_[BleSettingsCommandType::EndWrite].get());
    end_write_cmd->Initialize(transfer_buffer_, callbacks_.on_config_write);

    auto* start_write_cmd = static_cast<BleSettingsCommandStartWrite*>(
        commands_[BleSettingsCommandType::StartWrite].get());
    start_write_cmd->Initialize(transfer_buffer_->size());

    auto* request_read_cmd = static_cast<BleSettingsCommandRequestRead*>(
        commands_[BleSettingsCommandType::RequestRead].get());
    request_read_cmd->Initialize(callbacks_.on_config_read, callbacks_.on_send);
}

void BleSettingsCommandManager::Process(std::span<const uint8_t> data) {
    if(data.empty()) {
        LOG_ERR("Empty control command");
        return;
    }

    auto cmd = static_cast<BleSettingsCommandType>(data[0]);

    if(commands_.contains(cmd))
        commands_[cmd]->Process(data);
    else
        LOG_ERR("Unknown command: %d", static_cast<int>(cmd));
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command
