#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

namespace eerie_leap::domain::ui_domain::models {

using namespace std::string_view_literals;

enum class WidgetPropertyType : std::uint16_t {
    NONE = 0,
    TAGS,                   // array of int (enum)
    IS_VISIBLE,             // bool
    IS_ANIMATED,            // bool
    MIN_VALUE,              // float
    MAX_VALUE,              // float
    SENSOR_ID,              // string
    CHART_POINT_COUNT,      // int
    CHART_TYPE              // int (enum)
};

class WidgetProperty {
private:
    static constexpr const std::array WidgetPropertyTypeNames = {
        "NONE"sv,
        "TAGS"sv,
        "IS_VISIBLE"sv,
        "IS_ANIMATED"sv,
        "MIN_VALUE"sv,
        "MAX_VALUE"sv,
        "SENSOR_ID"sv,
        "CHART_POINT_COUNT"sv,
        "CHART_TYPE"sv
    };

public:
    static const char* GetTypeName(WidgetPropertyType type) {
        return WidgetPropertyTypeNames[static_cast<std::uint16_t>(type)].data();
    }

    static WidgetPropertyType GetType(const std::string& name) {
        for(size_t i = 0; i < size(WidgetPropertyTypeNames); ++i)
            if(WidgetPropertyTypeNames[i] == name)
                return static_cast<WidgetPropertyType>(i);

        throw std::runtime_error("Invalid widget property type.");
    }
};

} // namespace eerie_leap::domain::ui_domain::models
