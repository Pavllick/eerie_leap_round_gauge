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
        auto payload = std::make_unique<uint8_t[]>(data_len);
        memcpy(payload.get(), data, data_len);

        descriptors_.push_back({
            .type     = type,
            .data_len = data_len,
            .data     = payload.get(),
        });

        payloads_.push_back(std::move(payload));
        return *this;
    }

    BtDataBuilder& Add(const bt_data& entry) {
        return Add(entry.type, entry.data, entry.data_len);
    }

    AdBuffer Build() {
        return { std::move(descriptors_), std::move(payloads_) };
    }
};

} // namespace eerie_leap::subsys::bluetooth::utilities
