#pragma once

#include <boost/container/pmr/vector.hpp>
#include <boost/json.hpp>
#include <nameof.hpp>

#include "utilities/memory/memory_resource_manager.h"

namespace eerie_leap::configuration::json::configs {

namespace json = boost::json;
using namespace eerie_leap::utilities::memory;

// Property value types
struct JsonMapStringString {
    json::string key;
    json::string value;

    JsonMapStringString(json::storage_ptr sp = Mrm::GetBoostExtPmr())
        : key(sp), value(sp) {}
};

enum class JsonPropertyType {
    Int,
    Float,
    String,
    Bool,
    IntList,
    StringList,
    StringMap
};

struct JsonPropertyValue {
    JsonPropertyType type;
    json::value value;

    JsonPropertyValue(json::storage_ptr sp = Mrm::GetBoostExtPmr())
        : type(JsonPropertyType::Int), value(sp) {}
};

struct JsonPropertiesConfig {
    boost::container::pmr::vector<std::pair<json::string, JsonPropertyValue>> properties;

    JsonPropertiesConfig(json::storage_ptr sp = Mrm::GetBoostExtPmr())
        : properties(sp.get()) {}
};

struct JsonGridSettingsConfig {
    bool snap_enabled;
    int width;
    int height;
    int spacing_px;
};

struct JsonWidgetPositionConfig {
    int x;
    int y;
};

struct JsonWidgetSizeConfig {
    int width;
    int height;
};

struct JsonWidgetConfig {
    json::string type;
    int id;
    JsonWidgetPositionConfig position;
    JsonWidgetSizeConfig size;
    JsonPropertiesConfig properties;

    JsonWidgetConfig(json::storage_ptr sp = Mrm::GetBoostExtPmr())
        : type(sp), properties(sp) {}
};

struct JsonScreenConfig {
    int id;
    json::string type;
    JsonGridSettingsConfig grid;
    boost::container::pmr::vector<JsonWidgetConfig> widgets;

    JsonScreenConfig(json::storage_ptr sp = Mrm::GetBoostExtPmr())
        : type(sp), widgets(sp.get()) {}
};

struct JsonUiConfig {
    int version;
    int active_screen_index;
    JsonPropertiesConfig properties;
    boost::container::pmr::vector<JsonScreenConfig> screens;

    JsonUiConfig(json::storage_ptr sp = Mrm::GetBoostExtPmr())
        : properties(sp), screens(sp.get()) {}
};

// JSON serialization/deserialization for JsonMapStringString
static void tag_invoke(json::value_from_tag, json::value& jv, JsonMapStringString const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonMapStringString::key).c_str()] = config.key;
    obj[NAMEOF_MEMBER(&JsonMapStringString::value).c_str()] = config.value;

    jv = std::move(obj);
}

static JsonMapStringString tag_invoke(json::value_to_tag<JsonMapStringString>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    JsonMapStringString result;

    result.key = obj.at(NAMEOF_MEMBER(&JsonMapStringString::key).c_str()).as_string();
    result.value = obj.at(NAMEOF_MEMBER(&JsonMapStringString::value).c_str()).as_string();

    return result;
}

// JSON serialization/deserialization for JsonPropertiesConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonPropertiesConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    for(const auto& prop : config.properties) {
        json::object prop_obj(Mrm::GetBoostExtPmr());

        // Add type field
        json::string type_str(Mrm::GetBoostExtPmr());
        switch(prop.second.type) {
            case JsonPropertyType::Int:
                type_str = "int";
                break;
            case JsonPropertyType::Float:
                type_str = "float";
                break;
            case JsonPropertyType::String:
                type_str = "string";
                break;
            case JsonPropertyType::Bool:
                type_str = "bool";
                break;
            case JsonPropertyType::IntList:
                type_str = "int_list";
                break;
            case JsonPropertyType::StringList:
                type_str = "string_list";
                break;
            case JsonPropertyType::StringMap:
                type_str = "string_map";
                break;
        }
        prop_obj["type"] = std::move(type_str);
        prop_obj["value"] = prop.second.value;

        obj[prop.first.c_str()] = std::move(prop_obj);
    }

    jv = std::move(obj);
}

static JsonPropertiesConfig tag_invoke(json::value_to_tag<JsonPropertiesConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    JsonPropertiesConfig result;

    result.properties.reserve(obj.size());
    for(const auto& kv : obj) {
        json::string key(kv.key(), Mrm::GetBoostExtPmr());
        JsonPropertyValue val;

        const json::object& prop_obj = kv.value().as_object();

        // Parse type field
        json::string type_str = prop_obj.at("type").as_string();
        if(type_str == "int") {
            val.type = JsonPropertyType::Int;
        } else if(type_str == "float") {
            val.type = JsonPropertyType::Float;
        } else if(type_str == "string") {
            val.type = JsonPropertyType::String;
        } else if(type_str == "bool") {
            val.type = JsonPropertyType::Bool;
        } else if(type_str == "int_list") {
            val.type = JsonPropertyType::IntList;
        } else if(type_str == "string_list") {
            val.type = JsonPropertyType::StringList;
        } else if(type_str == "string_map") {
            val.type = JsonPropertyType::StringMap;
        }

        val.value = prop_obj.at("value");
        result.properties.push_back(std::make_pair(std::move(key), std::move(val)));
    }

    return result;
}

// JSON serialization/deserialization for JsonGridSettingsConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonGridSettingsConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonGridSettingsConfig::snap_enabled).c_str()] = config.snap_enabled;
    obj[NAMEOF_MEMBER(&JsonGridSettingsConfig::width).c_str()] = config.width;
    obj[NAMEOF_MEMBER(&JsonGridSettingsConfig::height).c_str()] = config.height;
    obj[NAMEOF_MEMBER(&JsonGridSettingsConfig::spacing_px).c_str()] = config.spacing_px;

    jv = std::move(obj);
}

static JsonGridSettingsConfig tag_invoke(json::value_to_tag<JsonGridSettingsConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    return {
        .snap_enabled = obj.at(NAMEOF_MEMBER(&JsonGridSettingsConfig::snap_enabled).c_str()).as_bool(),
        .width = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonGridSettingsConfig::width).c_str()).as_int64()),
        .height = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonGridSettingsConfig::height).c_str()).as_int64()),
        .spacing_px = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonGridSettingsConfig::spacing_px).c_str()).as_int64())
    };
}

// JSON serialization/deserialization for JsonWidgetPositionConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonWidgetPositionConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonWidgetPositionConfig::x).c_str()] = config.x;
    obj[NAMEOF_MEMBER(&JsonWidgetPositionConfig::y).c_str()] = config.y;

    jv = std::move(obj);
}

static JsonWidgetPositionConfig tag_invoke(json::value_to_tag<JsonWidgetPositionConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    return {
        .x = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonWidgetPositionConfig::x).c_str()).as_int64()),
        .y = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonWidgetPositionConfig::y).c_str()).as_int64())
    };
}

// JSON serialization/deserialization for JsonWidgetSizeConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonWidgetSizeConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonWidgetSizeConfig::width).c_str()] = config.width;
    obj[NAMEOF_MEMBER(&JsonWidgetSizeConfig::height).c_str()] = config.height;

    jv = std::move(obj);
}

static JsonWidgetSizeConfig tag_invoke(json::value_to_tag<JsonWidgetSizeConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    return {
        .width = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonWidgetSizeConfig::width).c_str()).as_int64()),
        .height = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonWidgetSizeConfig::height).c_str()).as_int64())
    };
}

// JSON serialization/deserialization for JsonWidgetConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonWidgetConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonWidgetConfig::type).c_str()] = config.type;
    obj[NAMEOF_MEMBER(&JsonWidgetConfig::id).c_str()] = config.id;
    obj[NAMEOF_MEMBER(&JsonWidgetConfig::position).c_str()] = json::value_from(config.position, Mrm::GetBoostExtPmr());
    obj[NAMEOF_MEMBER(&JsonWidgetConfig::size).c_str()] = json::value_from(config.size, Mrm::GetBoostExtPmr());
    obj[NAMEOF_MEMBER(&JsonWidgetConfig::properties).c_str()] = json::value_from(config.properties, Mrm::GetBoostExtPmr());

    jv = std::move(obj);
}

static JsonWidgetConfig tag_invoke(json::value_to_tag<JsonWidgetConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    JsonWidgetConfig result;

    result.type = obj.at(NAMEOF_MEMBER(&JsonWidgetConfig::type).c_str()).as_string();
    result.id = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonWidgetConfig::id).c_str()).as_int64());
    result.position = json::value_to<JsonWidgetPositionConfig>(obj.at(NAMEOF_MEMBER(&JsonWidgetConfig::position).c_str()));
    result.size = json::value_to<JsonWidgetSizeConfig>(obj.at(NAMEOF_MEMBER(&JsonWidgetConfig::size).c_str()));
    result.properties = json::value_to<JsonPropertiesConfig>(obj.at(NAMEOF_MEMBER(&JsonWidgetConfig::properties).c_str()));

    return result;
}

// JSON serialization/deserialization for JsonScreenConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonScreenConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonScreenConfig::id).c_str()] = config.id;
    obj[NAMEOF_MEMBER(&JsonScreenConfig::type).c_str()] = config.type;
    obj[NAMEOF_MEMBER(&JsonScreenConfig::grid).c_str()] = json::value_from(config.grid, Mrm::GetBoostExtPmr());

    json::array widgets_array(Mrm::GetBoostExtPmr());
    for(const auto& widget : config.widgets)
        widgets_array.push_back(json::value_from(widget, Mrm::GetBoostExtPmr()));
    obj[NAMEOF_MEMBER(&JsonScreenConfig::widgets).c_str()] = std::move(widgets_array);

    jv = std::move(obj);
}

static JsonScreenConfig tag_invoke(json::value_to_tag<JsonScreenConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    JsonScreenConfig result;

    result.id = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonScreenConfig::id).c_str()).as_int64());
    result.type = obj.at(NAMEOF_MEMBER(&JsonScreenConfig::type).c_str()).as_string();
    result.grid = json::value_to<JsonGridSettingsConfig>(obj.at(NAMEOF_MEMBER(&JsonScreenConfig::grid).c_str()));

    const json::array& widgets_array = obj.at(NAMEOF_MEMBER(&JsonScreenConfig::widgets).c_str()).as_array();
    result.widgets.reserve(widgets_array.size());
    for(const auto& elem : widgets_array)
        result.widgets.push_back(json::value_to<JsonWidgetConfig>(elem));

    return result;
}

// JSON serialization/deserialization for JsonUiConfig
static void tag_invoke(json::value_from_tag, json::value& jv, JsonUiConfig const& config) {
    jv.~value();
    new(&jv) json::value(json::object(Mrm::GetBoostExtPmr()));
    json::object& obj = jv.as_object();

    obj[NAMEOF_MEMBER(&JsonUiConfig::version).c_str()] = config.version;
    obj[NAMEOF_MEMBER(&JsonUiConfig::active_screen_index).c_str()] = config.active_screen_index;
    obj[NAMEOF_MEMBER(&JsonUiConfig::properties).c_str()] = json::value_from(config.properties, Mrm::GetBoostExtPmr());

    json::array screens_array(Mrm::GetBoostExtPmr());
    for(const auto& screen : config.screens)
        screens_array.push_back(json::value_from(screen, Mrm::GetBoostExtPmr()));
    obj[NAMEOF_MEMBER(&JsonUiConfig::screens).c_str()] = std::move(screens_array);

    jv = std::move(obj);
}

static JsonUiConfig tag_invoke(json::value_to_tag<JsonUiConfig>, json::value const& jv) {
    json::object const& obj = jv.as_object();
    JsonUiConfig result;

    result.version = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonUiConfig::version).c_str()).as_int64());
    result.active_screen_index = static_cast<int>(obj.at(NAMEOF_MEMBER(&JsonUiConfig::active_screen_index).c_str()).as_int64());
    result.properties = json::value_to<JsonPropertiesConfig>(obj.at(NAMEOF_MEMBER(&JsonUiConfig::properties).c_str()));

    const json::array& screens_array = obj.at(NAMEOF_MEMBER(&JsonUiConfig::screens).c_str()).as_array();
    result.screens.reserve(screens_array.size());
    for(const auto& elem : screens_array)
        result.screens.push_back(json::value_to<JsonScreenConfig>(elem));

    return result;
}

} // namespace eerie_leap::configuration::json::configs
