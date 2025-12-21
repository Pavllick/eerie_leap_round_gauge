/*
 * Generated using zcbor version 0.9.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 24
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "cbor_system_config_cbor_encode.h"
#include "zcbor_print.h"

#if DEFAULT_MAX_QTY != 24
#error "The type file was generated with a different default_max_qty than this file"
#endif

#define log_result(state, result, func) do { \
	if (!result) { \
		zcbor_trace_file(state); \
		zcbor_log("%s error: %s\r\n", func, zcbor_error_str(zcbor_peek_error(state))); \
	} else { \
		zcbor_log("%s success\r\n", func); \
	} \
} while(0)

static bool encode_CborSystemConfig(zcbor_state_t *state, const struct CborSystemConfig *input);


static bool encode_CborSystemConfig(
		zcbor_state_t *state, const struct CborSystemConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 5) && ((((zcbor_uint64_encode(state, (&(*input).device_id))))
	&& ((zcbor_uint32_encode(state, (&(*input).hw_version))))
	&& ((zcbor_uint32_encode(state, (&(*input).sw_version))))
	&& ((zcbor_uint32_encode(state, (&(*input).build_number))))
	&& ((zcbor_uint32_encode(state, (&(*input).interface_channel))))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 5))));

	log_result(state, res, __func__);
	return res;
}



int cbor_encode_CborSystemConfig(
		uint8_t *payload, size_t payload_len,
		const struct CborSystemConfig *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[3];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_CborSystemConfig, sizeof(states) / sizeof(zcbor_state_t), 1);
}
