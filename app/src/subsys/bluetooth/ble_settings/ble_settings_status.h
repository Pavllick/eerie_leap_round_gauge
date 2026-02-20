#pragma once

#include <span>
#include <cstdint>
#include <functional>

#include "ble_settings_service_enums.h"

namespace eerie_leap::subsys::bluetooth::ble_settings {

class BleSettingsStatus {
public:
    struct Status {
        BleSettingsState state;
        uint8_t settings_id;
        uint32_t transferred_bytes;
        uint32_t total_bytes;
        BleSettingsErrorCode error_code;
    } __attribute__((packed));

    using StateChangeHandler = std::function<void(BleSettingsState old_state, BleSettingsState new_state)>;

private:
    Status status_;
    BleSettingsState state_;
    StateChangeHandler on_state_change_;

public:
    BleSettingsStatus() = default;

    void Reset() noexcept {
        status_ = {};
        state_ = BleSettingsState::Idle;
    }

    [[nodiscard]] const Status& GetStatus() const noexcept {
        return status_;
    }

    [[nodiscard]] std::span<const uint8_t> GetStatusRaw() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&status_), sizeof(status_));
    }

    [[nodiscard]] BleSettingsState GetState() const noexcept {
        return state_;
    }

    void SetState(BleSettingsState new_state) noexcept {
        status_.state = new_state;
        state_ = new_state;

        if (on_state_change_)
            on_state_change_(state_, new_state);
    }

    void SetStateChangeHandler(StateChangeHandler handler) {
        on_state_change_ = handler;
    }

    [[nodiscard]] uint8_t GetSettingsId() const noexcept {
        return status_.settings_id;
    }

    void SetSettingsId(uint8_t settings_id) noexcept {
        status_.settings_id = settings_id;
    }

    [[nodiscard]] uint32_t GetTransferredBytes() const noexcept {
        return status_.transferred_bytes;
    }

    void SetTransferredBytes(uint32_t bytes) noexcept {
        status_.transferred_bytes = bytes;
    }

    [[nodiscard]] uint32_t GetTotalBytes() const noexcept {
        return status_.total_bytes;
    }

    void SetTotalBytes(uint32_t bytes) noexcept {
        status_.total_bytes = bytes;
    }

    [[nodiscard]] BleSettingsErrorCode GetErrorCode() const noexcept {
        return status_.error_code;
    }

    void SetErrorCode(BleSettingsErrorCode error_code) noexcept {
        status_.error_code = error_code;
    }
};

} // namespace eerie_leap::subsys::bluetooth::ble_settings
