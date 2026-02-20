#include "ble_settings_command_start_write.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandStartWrite::BleSettingsCommandStartWrite(std::shared_ptr<BleSettingsStatus> status)
    : BleSettingsCommandBase(status) {}

void BleSettingsCommandStartWrite::SetMaxTransferSize(size_t max_transfer_size) {
    max_transfer_size_ = max_transfer_size;
}

void BleSettingsCommandStartWrite::Process(std::span<const uint8_t> data) {
    if(data.size() < 6) {
        LOG_ERR("START_WRITE: insufficient data");
        status_->SetErrorCode(BleSettingsErrorCode::InsufficientData);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    auto type = static_cast<BleSettingsType>(data[1]);
    uint32_t size = data[2] | (data[3] << 8) |
                    (data[4] << 16) | (data[5] << 24);

    if(size > max_transfer_size_) {
        LOG_ERR("Config too large: %u bytes", size);
        status_->SetErrorCode(BleSettingsErrorCode::TransferTooLarge);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    status_->SetCurrentType(type);
    status_->SetTotalBytes(size);
    status_->SetTransferredBytes(0);
    status_->SetErrorCode(BleSettingsErrorCode::None);

    status_->SetState(BleSettingsState::Writing);

    LOG_INF("Starting write: type=%u, size=%u", static_cast<uint8_t>(type), size);
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command
