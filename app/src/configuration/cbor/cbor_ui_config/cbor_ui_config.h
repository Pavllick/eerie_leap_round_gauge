#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <zcbor_common.h>

#include <vector>
#include <variant>

#define UI_CONFIG_MAX_PROPERTIES_COUNT 40
#define UI_CONFIG_MAX_SCREEN_CONFIGURATIONS_COUNT 10
#define UI_CONFIG_MAX_WIDGET_CONFIGURATIONS_COUNT 20

struct map_tstrtstr {
	struct zcbor_string tstrtstr_key;
	struct zcbor_string tstrtstr;
};

using CborPropertyValueType = std::variant<
    std::monostate,
	int32_t,
    double,
    zcbor_string,
    bool,
    std::vector<int32_t>,
    std::vector<zcbor_string>,
    std::vector<map_tstrtstr>
>;

struct CborPropertyValueType_r {
	CborPropertyValueType value;

	enum {
		CborPropertyValueType_int_c,
		CborPropertyValueType_float_c,
		CborPropertyValueType_tstr_c,
		CborPropertyValueType_bool_c,
		CborPropertyValueType_int_l_c,
		CborPropertyValueType_tstr_l_c,
		CborPropertyValueType_map_c,
	} CborPropertyValueType_choice;
};

struct CborPropertiesConfig_CborPropertyValueType_m {
	zcbor_string CborPropertyValueType_m_key;
	CborPropertyValueType_r CborPropertyValueType_m;
};

struct CborPropertiesConfig {
	std::vector<CborPropertiesConfig_CborPropertyValueType_m> CborPropertyValueType_m;
};

struct CborGridSettingsConfig {
	bool snap_enabled;
	uint32_t width;
	uint32_t height;
	uint32_t spacing_px;
};

struct CborWidgetPositionConfig {
	int32_t x;
	int32_t y;
};

struct CborWidgetSizeConfig {
	uint32_t width;
	uint32_t height;
};

struct CborWidgetConfig {
	uint32_t type;
	uint32_t id;
	CborWidgetPositionConfig position;
	CborWidgetSizeConfig size;
	CborPropertiesConfig properties;
	bool properties_present;
};

struct CborScreenConfig {
	uint32_t id;
    uint32_t type;
	CborGridSettingsConfig grid;
	std::vector<CborWidgetConfig> CborWidgetConfig_m;
};

struct CborUiConfig {
	uint32_t version;
	uint32_t active_screen_index;
	CborPropertiesConfig properties;
	bool properties_present;
	std::vector<CborScreenConfig> CborScreenConfig_m;
};
