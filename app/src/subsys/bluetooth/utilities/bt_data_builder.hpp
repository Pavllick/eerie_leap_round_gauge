#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <zephyr/bluetooth/bluetooth.h>

#include "ad_buffer.hpp"

namespace eerie_leap::subsys::bluetooth::utilities {

class BtDataBuilder {
private:
    std::vector<bt_data> descriptors_;
    std::vector<std::unique_ptr<uint8_t[]>>  payloads_;

public:
    BtDataBuilder& Add(uint8_t type, const void* data, uint8_t data_len) {
        descriptors_.push_back({
            .type = type,
            .data_len = data_len,
            .data = static_cast<const uint8_t*>(data),
        });

        return *this;
    }

    BtDataBuilder& Add(uint8_t type, std::span<const uint8_t> data) {
        auto payload = std::make_unique<uint8_t[]>(data.size());
        memcpy(payload.get(), data.data(), data.size());

        payloads_.push_back(std::move(payload));

        return Add(type, payloads_.back().get(), data.size());
    }

    BtDataBuilder& Add(const bt_data& entry) {
        return Add(entry.type, entry.data, entry.data_len);
    }

    AdBuffer Build() {
        return { std::move(descriptors_), std::move(payloads_) };
    }
};

} // namespace eerie_leap::subsys::bluetooth::utilities
