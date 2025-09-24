#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "gauge_config.h"

int cbor_decode_GaugeConfig(
		const uint8_t *payload, size_t payload_len,
		struct GaugeConfig *result,
		size_t *payload_len_out);
