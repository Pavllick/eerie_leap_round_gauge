#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "cbor_ui_config.h"

int cbor_decode_CborUiConfig(
		const uint8_t *payload, size_t payload_len,
		struct CborUiConfig *result,
		size_t *payload_len_out);
