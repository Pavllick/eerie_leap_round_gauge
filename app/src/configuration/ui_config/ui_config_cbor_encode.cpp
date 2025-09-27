#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>
#include "zcbor_encode.h"
#include "ui_config_cbor_encode.h"
#include "zcbor_print.h"

#define log_result(state, result, func) do { \
	if (!result) { \
		zcbor_trace_file(state); \
		zcbor_log("%s error: %s\r\n", func, zcbor_error_str(zcbor_peek_error(state))); \
	} else { \
		zcbor_log("%s success\r\n", func); \
	} \
} while(0)

static bool encode_repeated_map_tstrtstr(zcbor_state_t *state, const struct map_tstrtstr *input);
static bool encode_PropertyValueType(zcbor_state_t *state, const struct PropertyValueType_r *input);
static bool encode_repeated_PropertiesConfig_PropertyValueType_m(zcbor_state_t *state, const struct PropertiesConfig_PropertyValueType_m *input);
static bool encode_PropertiesConfig(zcbor_state_t *state, const struct PropertiesConfig *input);
static bool encode_GridSettingsConfig(zcbor_state_t *state, const struct GridSettingsConfig *input);
static bool encode_WidgetPositionConfig(zcbor_state_t *state, const struct WidgetPositionConfig *input);
static bool encode_WidgetSizeConfig(zcbor_state_t *state, const struct WidgetSizeConfig *input);
static bool encode_WidgetConfig(zcbor_state_t *state, const struct WidgetConfig *input);
static bool encode_ScreenConfig(zcbor_state_t *state, const struct ScreenConfig *input);
static bool encode_UiConfig(zcbor_state_t *state, const struct UiConfig *input);

static bool encode_repeated_map_tstrtstr(
		zcbor_state_t *state, const struct map_tstrtstr *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((((zcbor_tstr_encode(state, (&(*input).tstrtstr_key))))
	&& (zcbor_tstr_encode(state, (&(*input).tstrtstr)))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_PropertyValueType(
		zcbor_state_t *state, const struct PropertyValueType_r *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_int_c) ? ((zcbor_int32_encode(state, &std::get<int32_t>(input->value))))
		: (((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_float_c) ? ((zcbor_float64_encode(state, &std::get<double>(input->value))))
		: (((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_tstr_c) ? ((zcbor_tstr_encode(state, &std::get<zcbor_string>(input->value))))
		: (((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_bool_c) ? ((zcbor_bool_encode(state, &std::get<bool>(input->value))))
		: (((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_int_l_c) ? ((zcbor_list_start_encode(state, 0) && ((zcbor_multi_encode(std::get<std::vector<int32_t>>(input->value).size(), (zcbor_encoder_t *)zcbor_int32_encode, state, std::get<std::vector<int32_t>>(input->value).data(), sizeof(int32_t))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0)))
		: (((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_tstr_l_c) ? ((zcbor_list_start_encode(state, 0) && ((zcbor_multi_encode(std::get<std::vector<zcbor_string>>(input->value).size(), (zcbor_encoder_t *)zcbor_tstr_encode, state, std::get<std::vector<zcbor_string>>(input->value).data(), sizeof(zcbor_string))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0)))
		: (((*input).PropertyValueType_choice == PropertyValueType_r::PropertyValueType_map_c) ? ((zcbor_map_start_encode(state, 0) && ((zcbor_multi_encode(std::get<std::vector<map_tstrtstr>>(input->value).size(), (zcbor_encoder_t *)encode_repeated_map_tstrtstr, state, std::get<std::vector<map_tstrtstr>>(input->value).data(), sizeof(map_tstrtstr))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 0)))
		: false))))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_repeated_PropertiesConfig_PropertyValueType_m(
		zcbor_state_t *state, const struct PropertiesConfig_PropertyValueType_m *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = ((((zcbor_tstr_encode(state, (&(*input).PropertyValueType_m_key))))
	&& (encode_PropertyValueType(state, (&(*input).PropertyValueType_m)))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_PropertiesConfig(
		zcbor_state_t *state, const struct PropertiesConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_map_start_encode(state, 0) && ((zcbor_multi_encode((*input).PropertyValueType_m_count, (zcbor_encoder_t *)encode_repeated_PropertiesConfig_PropertyValueType_m, state, input->PropertyValueType_m.data(), sizeof(struct PropertiesConfig_PropertyValueType_m))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_map_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_GridSettingsConfig(
		zcbor_state_t *state, const struct GridSettingsConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 0) && ((((zcbor_bool_encode(state, (&(*input).snap_enabled))))
	&& ((zcbor_uint32_encode(state, (&(*input).width))))
	&& ((zcbor_uint32_encode(state, (&(*input).height))))
	&& ((zcbor_uint32_encode(state, (&(*input).spacing_px))))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_WidgetPositionConfig(
		zcbor_state_t *state, const struct WidgetPositionConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 0) && ((((zcbor_int32_encode(state, (&(*input).x))))
	&& ((zcbor_int32_encode(state, (&(*input).y))))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_WidgetSizeConfig(
		zcbor_state_t *state, const struct WidgetSizeConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 0) && ((((zcbor_uint32_encode(state, (&(*input).width))))
	&& ((zcbor_uint32_encode(state, (&(*input).height))))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_WidgetConfig(
		zcbor_state_t *state, const struct WidgetConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 0) && ((((zcbor_uint32_encode(state, (&(*input).type))))
	&& ((zcbor_uint32_encode(state, (&(*input).id))))
	&& ((encode_WidgetPositionConfig(state, (&(*input).position))))
	&& ((encode_WidgetSizeConfig(state, (&(*input).size))))
	&& (!(*input).properties_present || encode_PropertiesConfig(state, (&(*input).properties)))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_ScreenConfig(
		zcbor_state_t *state, const struct ScreenConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 0) && ((((zcbor_uint32_encode(state, (&(*input).id))))
	&& ((encode_GridSettingsConfig(state, (&(*input).grid))))
	&& ((zcbor_list_start_encode(state, 0) && ((zcbor_multi_encode((*input).WidgetConfig_m_count, (zcbor_encoder_t *)encode_WidgetConfig, state, input->WidgetConfig_m.data(), sizeof(struct WidgetConfig))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0)))
	) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}

static bool encode_UiConfig(
		zcbor_state_t *state, const struct UiConfig *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 0) && ((((zcbor_uint32_encode(state, (&(*input).version))))
	&& ((zcbor_uint32_encode(state, (&(*input).active_screen_index))))
	&& (!(*input).properties_present || encode_PropertiesConfig(state, (&(*input).properties)))
	&& ((zcbor_list_start_encode(state, 0) && ((zcbor_multi_encode((*input).ScreenConfig_m_count, (zcbor_encoder_t *)encode_ScreenConfig, state, input->ScreenConfig_m.data(), sizeof(struct ScreenConfig))
	) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0)))
	) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 0))));

	log_result(state, res, __func__);
	return res;
}



int cbor_encode_UiConfig(
		uint8_t *payload, size_t payload_len,
		const struct UiConfig *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[10];

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)encode_UiConfig, sizeof(states) / sizeof(zcbor_state_t), 1);
}
