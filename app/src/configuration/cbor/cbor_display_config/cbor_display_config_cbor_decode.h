/*
 * Generated using zcbor version 0.9.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 24
 */

#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>

#include "cbor_display_config.h"

int cbor_decode_CborDisplayConfig(
                const uint8_t *payload, size_t payload_len,
                struct CborDisplayConfig *result,
                size_t *payload_len_out);
