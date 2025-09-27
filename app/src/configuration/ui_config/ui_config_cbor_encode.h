#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "ui_config.h"

int cbor_encode_UiConfig(
		uint8_t *payload, size_t payload_len,
		const struct UiConfig *input,
		size_t *payload_len_out);
