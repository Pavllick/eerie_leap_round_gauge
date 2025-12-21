#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "zcbor_decode.h"
#include "cbor_ui_config_cbor_decode.h"
#include "zcbor_print.h"

#include <zephyr/kernel.h>
#include "utilities/memory/heap_allocator.h"

using namespace eerie_leap::utilities::memory;

#define log_result(state, result, func) do { \
	if (!result) { \
		zcbor_trace_file(state); \
		zcbor_log("%s error: %s\r\n", func, zcbor_error_str(zcbor_peek_error(state))); \
	} else { \
		zcbor_log("%s success\r\n", func); \
	} \
} while(0)

static bool decode_repeated_map_tstrtstr(zcbor_state_t *state, struct map_tstrtstr *result);
static bool decode_CborPropertyValueType(zcbor_state_t *state, struct CborPropertyValueType_r *result);
static bool decode_repeated_CborPropertiesConfig_CborPropertyValueType_m(zcbor_state_t *state, struct CborPropertiesConfig_CborPropertyValueType_m *result);
static bool decode_CborPropertiesConfig(zcbor_state_t *state, struct CborPropertiesConfig *result);
static bool decode_CborGridSettingsConfig(zcbor_state_t *state, struct CborGridSettingsConfig *result);
static bool decode_CborWidgetPositionConfig(zcbor_state_t *state, struct CborWidgetPositionConfig *result);
static bool decode_CborWidgetSizeConfig(zcbor_state_t *state, struct CborWidgetSizeConfig *result);
static bool decode_CborWidgetConfig(zcbor_state_t *state, struct CborWidgetConfig *result);
static bool decode_CborScreenConfig(zcbor_state_t *state, struct CborScreenConfig *result);
static bool decode_CborUiConfig(zcbor_state_t *state, struct CborUiConfig *result);


static bool decode_repeated_map_tstrtstr(
		zcbor_state_t *state, struct map_tstrtstr *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((((zcbor_tstr_decode(state, (&(*result).tstrtstr_key))))
	&& (zcbor_tstr_decode(state, (&(*result).tstrtstr)))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborPropertyValueType(
		zcbor_state_t *state, struct CborPropertyValueType_r *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = false;

	// Try to decode each variant type in order
	// Save the current state to backtrack if needed
	zcbor_state_t backup_state = *state;

	// Try int32_t
	{
		int32_t val;
		if (zcbor_int32_decode(state, &val)) {
			result->value = val;
			result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_int_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try double
		double val;
		if (zcbor_float64_decode(state, &val)) {
			result->value = val;
			result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_float_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try string
		zcbor_string val;
		if (zcbor_tstr_decode(state, &val)) {
			result->value = val;
			result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_tstr_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try bool
		bool val;
		if (zcbor_bool_decode(state, &val)) {
			result->value = val;
			result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_bool_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try list of int32_t
		if (zcbor_list_start_decode(state)) {
			auto buffer = make_shared_ext<ExtVector>(sizeof(int32_t) * UI_CONFIG_MAX_PROPERTIES_COUNT);
			size_t count = 0;

			if (zcbor_multi_decode(0, UI_CONFIG_MAX_PROPERTIES_COUNT, &count, (zcbor_decoder_t*)zcbor_int32_decode,
					state, buffer->data(), sizeof(int32_t)) && zcbor_list_end_decode(state)) {

				std::vector<int32_t> vec(count);
				memcpy(vec.data(), buffer->data(), count * sizeof(int32_t));
				result->value = std::move(vec);
				result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_int_l_c;
				res = true;
			}
		}
	}

	if (!res) {
		*state = backup_state;
		// Try list of strings
		if (zcbor_list_start_decode(state)) {
			auto buffer = make_shared_ext<ExtVector>(sizeof(zcbor_string) * UI_CONFIG_MAX_PROPERTIES_COUNT);
			size_t count = 0;

			if (zcbor_multi_decode(0, UI_CONFIG_MAX_PROPERTIES_COUNT, &count, (zcbor_decoder_t*)zcbor_tstr_decode,
				state, buffer->data(), sizeof(zcbor_string)) && zcbor_list_end_decode(state)) {

				std::vector<zcbor_string> vec(count);
				memcpy(vec.data(), buffer->data(), count * sizeof(zcbor_string));
				result->value = std::move(vec);
				result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_tstr_l_c;
				res = true;
			}
		}
	}

	if (!res) {
		*state = backup_state;
		// Try map (vector of map_tstrtstr)
		if (zcbor_map_start_decode(state)) {
			auto buffer = make_shared_ext<ExtVector>(sizeof(map_tstrtstr) * UI_CONFIG_MAX_PROPERTIES_COUNT);
			size_t count = 0;

			if (zcbor_multi_decode(0, UI_CONFIG_MAX_PROPERTIES_COUNT, &count, (zcbor_decoder_t*)decode_repeated_map_tstrtstr,
				state, buffer->data(), sizeof(map_tstrtstr)) && zcbor_map_end_decode(state)) {

				std::vector<map_tstrtstr> vec(count);
				memcpy(vec.data(), buffer->data(), count * sizeof(map_tstrtstr));
				result->value = std::move(vec);
				result->CborPropertyValueType_choice = CborPropertyValueType_r::CborPropertyValueType_map_c;
				res = true;
			}
		}
	}

	log_result(state, res, __func__);
	return res;
}

static bool decode_repeated_CborPropertiesConfig_CborPropertyValueType_m(
		zcbor_state_t *state, struct CborPropertiesConfig_CborPropertyValueType_m *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((zcbor_tstr_decode(state, (&(*result).CborPropertyValueType_m_key))))
		&& (decode_CborPropertyValueType(state, (&(*result).CborPropertyValueType_m)));

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborPropertiesConfig(
		zcbor_state_t *state, struct CborPropertiesConfig *result)
{
	zcbor_log("%s\r\n", __func__);

    size_t CborPropertyValueType_m_count = 0;
	result->CborPropertyValueType_m.clear();

	auto buffer = make_shared_ext<ExtVector>(sizeof(CborPropertiesConfig_CborPropertyValueType_m) * UI_CONFIG_MAX_PROPERTIES_COUNT);

	bool res = zcbor_map_start_decode(state)
		&& ((zcbor_multi_decode(0, UI_CONFIG_MAX_PROPERTIES_COUNT, &CborPropertyValueType_m_count, (zcbor_decoder_t *)decode_repeated_CborPropertiesConfig_CborPropertyValueType_m, state, buffer->data(), sizeof(struct CborPropertiesConfig_CborPropertyValueType_m))) ||
			(zcbor_list_map_end_force_decode(state), false))
		&& zcbor_map_end_decode(state);

	if (res) {
		result->CborPropertyValueType_m.resize(CborPropertyValueType_m_count);
		memcpy(result->CborPropertyValueType_m.data(), buffer->data(),
			CborPropertyValueType_m_count * sizeof(struct CborPropertiesConfig_CborPropertyValueType_m));
	}

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborGridSettingsConfig(
		zcbor_state_t *state, struct CborGridSettingsConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_bool_decode(state, (&(*result).snap_enabled))))
	&& ((zcbor_uint32_decode(state, (&(*result).width))))
	&& ((zcbor_uint32_decode(state, (&(*result).height))))
	&& ((zcbor_uint32_decode(state, (&(*result).spacing_px))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborWidgetPositionConfig(
		zcbor_state_t *state, struct CborWidgetPositionConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_int32_decode(state, (&(*result).x))))
	&& ((zcbor_int32_decode(state, (&(*result).y))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborWidgetSizeConfig(
		zcbor_state_t *state, struct CborWidgetSizeConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).width))))
	&& ((zcbor_uint32_decode(state, (&(*result).height))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborWidgetConfig(
		zcbor_state_t *state, struct CborWidgetConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).type))))
	&& ((zcbor_uint32_decode(state, (&(*result).id))))
	&& ((decode_CborWidgetPositionConfig(state, (&(*result).position))))
	&& ((decode_CborWidgetSizeConfig(state, (&(*result).size))))
	&& ((*result).properties_present = ((decode_CborPropertiesConfig(state, (&(*result).properties)))), 1)) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborScreenConfig(
		zcbor_state_t *state, struct CborScreenConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	auto buffer = make_shared_ext<ExtVector>(sizeof(CborWidgetConfig) * UI_CONFIG_MAX_WIDGET_CONFIGURATIONS_COUNT);
    size_t CborWidgetConfig_m_count = 0;

	bool res = (((zcbor_list_start_decode(state) && ((
       ((zcbor_uint32_decode(state, (&(*result).id))))
    && ((zcbor_uint32_decode(state, (&(*result).type))))
	&& ((decode_CborGridSettingsConfig(state, (&(*result).grid))))
	&& ((zcbor_list_start_decode(state) && ((zcbor_multi_decode(0, UI_CONFIG_MAX_WIDGET_CONFIGURATIONS_COUNT, &CborWidgetConfig_m_count, (zcbor_decoder_t *)decode_CborWidgetConfig, state, buffer->data(), sizeof(struct CborWidgetConfig))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state)))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	if (res) {
		result->CborWidgetConfig_m.resize(CborWidgetConfig_m_count);
		memcpy(result->CborWidgetConfig_m.data(), buffer->data(),
			CborWidgetConfig_m_count * sizeof(struct CborWidgetConfig));
	}

	log_result(state, res, __func__);
	return res;
}

static bool decode_CborUiConfig(
		zcbor_state_t *state, struct CborUiConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	auto buffer = make_shared_ext<ExtVector>(sizeof(CborScreenConfig) * UI_CONFIG_MAX_SCREEN_CONFIGURATIONS_COUNT);
    size_t CborScreenConfig_m_count = 0;

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).version))))
	&& ((zcbor_uint32_decode(state, (&(*result).active_screen_index))))
	&& ((*result).properties_present = ((decode_CborPropertiesConfig(state, (&(*result).properties)))), 1)
	&& ((zcbor_list_start_decode(state) && ((zcbor_multi_decode(0, UI_CONFIG_MAX_SCREEN_CONFIGURATIONS_COUNT, &CborScreenConfig_m_count, (zcbor_decoder_t *)decode_CborScreenConfig, state, buffer->data(), sizeof(struct CborScreenConfig))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))))
	|| (zcbor_list_map_end_force_decode(state), false))) && zcbor_list_end_decode(state)));

	if (res) {
		result->CborScreenConfig_m.resize(CborScreenConfig_m_count);
		memcpy(result->CborScreenConfig_m.data(), buffer->data(),
			CborScreenConfig_m_count * sizeof(struct CborScreenConfig));
	}

	log_result(state, res, __func__);
	return res;
}



int cbor_decode_CborUiConfig(
		const uint8_t *payload, size_t payload_len,
		struct CborUiConfig *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[10];

	return zcbor_entry_function(payload, payload_len, (void *)result, payload_len_out, states,
		(zcbor_decoder_t *)decode_CborUiConfig, sizeof(states) / sizeof(zcbor_state_t), 1);
}
