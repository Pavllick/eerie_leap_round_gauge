#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "ui_config.h"

int cbor_decode_UiConfig(
		const uint8_t *payload, size_t payload_len,
		struct UiConfig *result,
		size_t *payload_len_out);
