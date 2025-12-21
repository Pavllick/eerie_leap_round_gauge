#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_instance.h>
#include <zcbor_common.h>
#include <zcbor_encode.h>
#include <zcbor_decode.h>

#include "utilities/memory/memory_resource_manager.h"

namespace eerie_leap::utilities::cbor {

using namespace eerie_leap::utilities::memory;

template <typename T>
class CborSerializer {
public:
    using EncodeFn = int (*)(uint8_t*, size_t, const T*, size_t*);
    using DecodeFn = int (*)(const uint8_t*, size_t, T*, size_t*);
    using GetSerializingSizeFn = size_t (*)(const T&);

    CborSerializer(EncodeFn encoder, DecodeFn decoder, GetSerializingSizeFn getSerializingSizeFn)
        : encodeFn_(encoder), decodeFn_(decoder), getSerializingSizeFn_(getSerializingSizeFn) {}

    std::pmr::vector<uint8_t> Serialize(const T& obj, size_t *payload_len_out = nullptr) {
        LOG_MODULE_DECLARE(cbor_serializer_logger);

        std::pmr::vector<uint8_t> buffer(getSerializingSizeFn_(obj), Mrm::GetExtPmr());

        size_t obj_size = 0;
        if(encodeFn_(buffer.data(), buffer.size(), &obj, &obj_size)) {
            LOG_ERR("Failed to encode object.");
            return {};
        }

        if (payload_len_out != nullptr)
            *payload_len_out = obj_size;

        return buffer;
    }

    pmr_unique_ptr<T> Deserialize(std::span<const uint8_t> input) {
        LOG_MODULE_DECLARE(cbor_serializer_logger);

        auto obj = make_unique_pmr<T>(Mrm::GetExtPmr());
        if(decodeFn_(input.data(), input.size(), obj.get(), nullptr)) {
            LOG_ERR("Failed to decode object.");
            return nullptr;
        }

        return obj;
    }

private:
    EncodeFn encodeFn_;
    DecodeFn decodeFn_;
    GetSerializingSizeFn getSerializingSizeFn_;
};

} // namespace eerie_leap::utilities::cbor
