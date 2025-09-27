#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "zcbor_decode.h"
#include "ui_config_cbor_decode.h"
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
static bool decode_PropertyValueType(zcbor_state_t *state, struct PropertyValueType_r *result);
static bool decode_repeated_PropertiesConfig_PropertyValueType_m(zcbor_state_t *state, struct PropertiesConfig_PropertyValueType_m *result);
static bool decode_PropertiesConfig(zcbor_state_t *state, struct PropertiesConfig *result);
static bool decode_GridSettingsConfig(zcbor_state_t *state, struct GridSettingsConfig *result);
static bool decode_WidgetPositionConfig(zcbor_state_t *state, struct WidgetPositionConfig *result);
static bool decode_WidgetSizeConfig(zcbor_state_t *state, struct WidgetSizeConfig *result);
static bool decode_WidgetConfig(zcbor_state_t *state, struct WidgetConfig *result);
static bool decode_ScreenConfig(zcbor_state_t *state, struct ScreenConfig *result);
static bool decode_UiConfig(zcbor_state_t *state, struct UiConfig *result);


static bool decode_repeated_map_tstrtstr(
		zcbor_state_t *state, struct map_tstrtstr *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((((zcbor_tstr_decode(state, (&(*result).tstrtstr_key))))
	&& (zcbor_tstr_decode(state, (&(*result).tstrtstr)))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_PropertyValueType(
		zcbor_state_t *state, struct PropertyValueType_r *result)
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
			result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try double
		double val;
		if (zcbor_float64_decode(state, &val)) {
			result->value = val;
			result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_float_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try string
		zcbor_string val;
		if (zcbor_tstr_decode(state, &val)) {
			result->value = val;
			result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_c;
			res = true;
		}
	}

	if (!res) {
		*state = backup_state;
		// Try bool
		bool val;
		if (zcbor_bool_decode(state, &val)) {
			result->value = val;
			result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_bool_c;
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
				result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_int_l_c;
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
				result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_tstr_l_c;
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
				result->PropertyValueType_choice = PropertyValueType_r::PropertyValueType_map_c;
				res = true;
			}
		}
	}

	log_result(state, res, __func__);
	return res;
}

static bool decode_repeated_PropertiesConfig_PropertyValueType_m(
		zcbor_state_t *state, struct PropertiesConfig_PropertyValueType_m *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((zcbor_tstr_decode(state, (&(*result).PropertyValueType_m_key))))
		&& (decode_PropertyValueType(state, (&(*result).PropertyValueType_m)));

	log_result(state, res, __func__);
	return res;
}

static bool decode_PropertiesConfig(
		zcbor_state_t *state, struct PropertiesConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	result->PropertyValueType_m_count = 0;
	result->PropertyValueType_m.clear();

	auto buffer = make_shared_ext<ExtVector>(sizeof(PropertiesConfig_PropertyValueType_m) * UI_CONFIG_MAX_PROPERTIES_COUNT);

	bool res = zcbor_map_start_decode(state)
		&& ((zcbor_multi_decode(0, UI_CONFIG_MAX_PROPERTIES_COUNT, &(*result).PropertyValueType_m_count, (zcbor_decoder_t *)decode_repeated_PropertiesConfig_PropertyValueType_m, state, buffer->data(), sizeof(struct PropertiesConfig_PropertyValueType_m))) ||
			(zcbor_list_map_end_force_decode(state), false))
		&& zcbor_map_end_decode(state);

	if (res) {
		result->PropertyValueType_m.resize(result->PropertyValueType_m_count);
		memcpy(result->PropertyValueType_m.data(), buffer->data(),
			result->PropertyValueType_m_count * sizeof(struct PropertiesConfig_PropertyValueType_m));
	}

	log_result(state, res, __func__);
	return res;
}

static bool decode_GridSettingsConfig(
		zcbor_state_t *state, struct GridSettingsConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_bool_decode(state, (&(*result).snap_enabled))))
	&& ((zcbor_uint32_decode(state, (&(*result).width))))
	&& ((zcbor_uint32_decode(state, (&(*result).height))))
	&& ((zcbor_uint32_decode(state, (&(*result).spacing_px))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_WidgetPositionConfig(
		zcbor_state_t *state, struct WidgetPositionConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_int32_decode(state, (&(*result).x))))
	&& ((zcbor_int32_decode(state, (&(*result).y))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_WidgetSizeConfig(
		zcbor_state_t *state, struct WidgetSizeConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).width))))
	&& ((zcbor_uint32_decode(state, (&(*result).height))))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_WidgetConfig(
		zcbor_state_t *state, struct WidgetConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).type))))
	&& ((zcbor_uint32_decode(state, (&(*result).id))))
	&& ((decode_WidgetPositionConfig(state, (&(*result).position))))
	&& ((decode_WidgetSizeConfig(state, (&(*result).size))))
	&& ((*result).properties_present = ((decode_PropertiesConfig(state, (&(*result).properties)))), 1)) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	log_result(state, res, __func__);
	return res;
}

static bool decode_ScreenConfig(
		zcbor_state_t *state, struct ScreenConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	auto buffer = make_shared_ext<ExtVector>(sizeof(WidgetConfig) * UI_CONFIG_MAX_WIDGET_CONFIGURATIONS_COUNT);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).id))))
	&& ((decode_GridSettingsConfig(state, (&(*result).grid))))
	&& ((zcbor_list_start_decode(state) && ((zcbor_multi_decode(0, UI_CONFIG_MAX_WIDGET_CONFIGURATIONS_COUNT, &(*result).WidgetConfig_m_count, (zcbor_decoder_t *)decode_WidgetConfig, state, buffer->data(), sizeof(struct WidgetConfig))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state)))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))));

	if (res) {
		result->WidgetConfig_m.resize(result->WidgetConfig_m_count);
		memcpy(result->WidgetConfig_m.data(), buffer->data(),
			result->WidgetConfig_m_count * sizeof(struct WidgetConfig));
	}

	log_result(state, res, __func__);
	return res;
}

static bool decode_UiConfig(
		zcbor_state_t *state, struct UiConfig *result)
{
	zcbor_log("%s\r\n", __func__);

	auto buffer = make_shared_ext<ExtVector>(sizeof(ScreenConfig) * UI_CONFIG_MAX_SCREEN_CONFIGURATIONS_COUNT);

	bool res = (((zcbor_list_start_decode(state) && ((((zcbor_uint32_decode(state, (&(*result).version))))
	&& ((zcbor_uint32_decode(state, (&(*result).active_screen_index))))
	&& ((*result).properties_present = ((decode_PropertiesConfig(state, (&(*result).properties)))), 1)
	&& ((zcbor_list_start_decode(state) && ((zcbor_multi_decode(0, UI_CONFIG_MAX_SCREEN_CONFIGURATIONS_COUNT, &(*result).ScreenConfig_m_count, (zcbor_decoder_t *)decode_ScreenConfig, state, buffer->data(), sizeof(struct ScreenConfig))) || (zcbor_list_map_end_force_decode(state), false)) && zcbor_list_end_decode(state))))
	|| (zcbor_list_map_end_force_decode(state), false))) && zcbor_list_end_decode(state)));

	if (res) {
		result->ScreenConfig_m.resize(result->ScreenConfig_m_count);
		memcpy(result->ScreenConfig_m.data(), buffer->data(),
			result->ScreenConfig_m_count * sizeof(struct ScreenConfig));
	}

	log_result(state, res, __func__);
	return res;
}



int cbor_decode_UiConfig(
		const uint8_t *payload, size_t payload_len,
		struct UiConfig *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[10];

	return zcbor_entry_function(payload, payload_len, (void *)result, payload_len_out, states,
		(zcbor_decoder_t *)decode_UiConfig, sizeof(states) / sizeof(zcbor_state_t), 1);
}
