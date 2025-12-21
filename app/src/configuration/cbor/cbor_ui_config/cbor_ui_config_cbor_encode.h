#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "cbor_ui_config.h"

int cbor_encode_CborUiConfig(
		uint8_t *payload, size_t payload_len,
		const struct CborUiConfig *input,
		size_t *payload_len_out);
