#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <zcbor_common.h>

#include <vector>
#include <variant>

#define GAUGE_CONFIG_MAX_PROPERTIES_COUNT 40
#define GAUGE_CONFIG_MAX_SCREEN_CONFIGURATIONS_COUNT 10
#define GAUGE_CONFIG_MAX_WIDGET_CONFIGURATIONS_COUNT 20

struct map_tstrtstr {
	struct zcbor_string tstrtstr_key;
	struct zcbor_string tstrtstr;
};

using PropertyValueType = std::variant<
	int32_t,
    double,
    zcbor_string,
    bool,
    std::vector<int32_t>,
    std::vector<zcbor_string>,
    std::vector<map_tstrtstr>
>;

struct PropertyValueType_r {
	PropertyValueType value;

	enum {
		PropertyValueType_int_c,
		PropertyValueType_float_c,
		PropertyValueType_tstr_c,
		PropertyValueType_bool_c,
		PropertyValueType_int_l_c,
		PropertyValueType_tstr_l_c,
		PropertyValueType_map_c,
	} PropertyValueType_choice;
};

struct PropertiesConfig_PropertyValueType_m {
	zcbor_string PropertyValueType_m_key;
	PropertyValueType_r PropertyValueType_m;
};

struct PropertiesConfig {
	std::vector<PropertiesConfig_PropertyValueType_m> PropertyValueType_m;
	size_t PropertyValueType_m_count;
};

struct GridSettingsConfig {
	bool snap_enabled;
	uint32_t width;
	uint32_t height;
	uint32_t spacing_px;
};

struct WidgetPositionConfig {
	int32_t x;
	int32_t y;
};

struct WidgetSizeConfig {
	uint32_t width;
	uint32_t height;
};

struct WidgetConfig {
	uint32_t type;
	uint32_t id;
	WidgetPositionConfig position;
	WidgetSizeConfig size;
	bool is_animation_enabled;
	PropertiesConfig properties;
	bool properties_present;
};

struct ScreenConfig {
	uint32_t id;
	GridSettingsConfig grid;
	std::vector<WidgetConfig> WidgetConfig_m;
	size_t WidgetConfig_m_count;
};

struct GaugeConfig {
	uint32_t version;
	uint32_t active_screen_index;
	PropertiesConfig properties;
	bool properties_present;
	std::vector<ScreenConfig> ScreenConfig_m;
	size_t ScreenConfig_m_count;
};
