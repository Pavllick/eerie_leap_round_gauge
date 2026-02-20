#pragma once

#include <cstdint>
#include <span>
#include <memory>

#include <zephyr/logging/log.h>

#include "../ble_settings_status.h"
#include "i_ble_settings_command_request.h"

namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command {

class BleSettingsCommandRequestBase : public IBleSettingsCommandRequest {
protected:
    std::shared_ptr<BleSettingsStatus> status_;

public:
    explicit BleSettingsCommandRequestBase(std::shared_ptr<BleSettingsStatus> status)
        : status_(status) {}
    virtual ~BleSettingsCommandRequestBase() = default;
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings::ble_settings_command
