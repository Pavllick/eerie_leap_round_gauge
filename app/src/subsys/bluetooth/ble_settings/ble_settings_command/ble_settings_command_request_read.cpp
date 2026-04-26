#include "ble_settings_command_request_read.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandRequestRead::BleSettingsCommandRequestRead(
    std::shared_ptr<BleSettingsStatus> status)
        : BleSettingsCommandRequestBase(status) {}

void BleSettingsCommandRequestRead::Initialize(const ReadHandler& handler, const SendHandler& send_handler) {
    read_handler_ = handler;
    send_handler_ = send_handler;
}

void BleSettingsCommandRequestRead::Process(std::span<const uint8_t> data) {
    if(status_->GetState() != BleSettingsState::Idle) {
        LOG_ERR("RequestRead: not in Idle state");
        status_->SetErrorCode(BleSettingsErrorCode::InvalidState);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    if(data.size() < 2) {
        LOG_ERR("RequestRead: insufficient data");
        status_->SetErrorCode(BleSettingsErrorCode::InsufficientData);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    uint8_t settings_id = data[1];

    if(!read_handler_) {
        LOG_ERR("RequestRead: no read handler registered");
        status_->SetErrorCode(BleSettingsErrorCode::HandlerFailed);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    if(!send_handler_) {
        LOG_ERR("RequestRead: no send handler registered");
        status_->SetErrorCode(BleSettingsErrorCode::HandlerFailed);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    // Transition to Reading state BEFORE callback
    status_->SetSettingsId(settings_id);
    status_->SetState(BleSettingsState::Reading);
    status_->SetErrorCode(BleSettingsErrorCode::None);

    LOG_INF("RequestRead: settings_id=%u", settings_id);

    std::span<const uint8_t> config_data = read_handler_(settings_id);

    // Check if state changed during callback (disconnect, abort, etc.)
    if(status_->GetState() != BleSettingsState::Reading) {
        LOG_INF("RequestRead: state changed during callback, aborting");
        return;
    }

    if(config_data.empty()) {
        LOG_ERR("RequestRead: handler returned empty configuration data");
        status_->SetErrorCode(BleSettingsErrorCode::HandlerFailed);
        status_->SetState(BleSettingsState::Error);
        return;
    }

    bool success = send_handler_(settings_id, config_data);

    if(!success) {
        LOG_ERR("RequestRead: send failed");
        // Send already updated state to Error
    }
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command
