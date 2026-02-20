#include "ble_settings_command_request_read.h"

LOG_MODULE_DECLARE(ble_settings_logger);

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

BleSettingsCommandRequestRead::BleSettingsCommandRequestRead(std::shared_ptr<BleSettingsStatus> status)
    : BleSettingsCommandBase(status) {}

void BleSettingsCommandRequestRead::Process(std::span<const uint8_t> data) {
    if(data.size() < 2) {
        LOG_ERR("REQUEST_READ: insufficient data");
        return;
    }

    auto type = static_cast<BleSettingsType>(data[1]);
    LOG_INF("Read requested: type=%u", static_cast<uint8_t>(type));

    // TODO: Remove from here and implement BTE read callback
    // SendConfig(conn, type);
}

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command
