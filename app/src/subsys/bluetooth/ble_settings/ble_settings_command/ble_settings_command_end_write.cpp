#include "ble_settings_command_end_write.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandEndWrite::BleSettingsCommandEndWrite(
    std::shared_ptr<BleSettingsStatus> status)
        : BleSettingsCommandRequestBase(status) {}

void BleSettingsCommandEndWrite::Initialize(
    std::shared_ptr<std::pmr::vector<uint8_t>> transfer_buffer,
    const WriteHandler& write_handler) {

    transfer_buffer_ = std::move(transfer_buffer);
    write_handler_ = write_handler;
}

void BleSettingsCommandEndWrite::Process(std::span<const uint8_t> data) {
    if(status_->GetState() != BleSettingsState::Writing) {
        LOG_ERR("END_WRITE: not in writing state");
        status_->SetErrorCode(BleSettingsErrorCode::InvalidState);
        status_->SetState(BleSettingsState::Error);

        return;
    }

    if(status_->GetTransferredBytes() != status_->GetTotalBytes()) {
        LOG_ERR("Incomplete transfer: %u/%u bytes",
            status_->GetTransferredBytes(), status_->GetTotalBytes());
        status_->SetErrorCode(BleSettingsErrorCode::IncompleteTransfer);
        status_->SetState(BleSettingsState::Error);

        return;
    }

    if(write_handler_) {
        auto received_data = std::span(transfer_buffer_->data(), status_->GetTransferredBytes());
        bool success = write_handler_(status_->GetSettingsId(), received_data);

        // A disconnect (or another command) may have fired while the
        // lock was released and already reset the state machine.  Only
        // act on the callback result if we are still in Writing.
        if(status_->GetState() != BleSettingsState::Writing) {
            LOG_INF("END_WRITE: state changed during callback, ignoring result");
            return;
        }

        if(success) {
            LOG_INF("Config write successful");
            status_->Reset();
        } else {
            LOG_ERR("Config write handler failed");
            status_->SetErrorCode(BleSettingsErrorCode::HandlerFailed);
            status_->SetState(BleSettingsState::Error);
        }
    } else {
        LOG_WRN("No write handler registered");
        status_->Reset();
    }
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command
