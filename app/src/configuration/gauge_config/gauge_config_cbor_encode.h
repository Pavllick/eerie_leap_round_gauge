#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "gauge_config.h"

int cbor_encode_GaugeConfig(
		uint8_t *payload, size_t payload_len,
		const struct GaugeConfig *input,
		size_t *payload_len_out);
