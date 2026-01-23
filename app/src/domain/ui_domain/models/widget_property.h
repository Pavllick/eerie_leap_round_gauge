#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

namespace eerie_leap::domain::ui_domain::models {

using namespace std::string_view_literals;

enum class WidgetPropertyType : std::uint16_t {
    NONE = 0,
    IS_VISIBLE,             // bool
    IS_ACTIVE,              // bool
    IS_SMOOTHED,            // bool
    MIN_VALUE,              // float
    MAX_VALUE,              // float
    SENSOR_ID,              // string
    CHART_POINT_COUNT,      // int
    CHART_TYPE,             // int (enum)
    UI_EVENT_TYPE,          // UiEventType
    LABEL,                  // string
    VALUE_PRECISION,        // int
    EDGE_OFFSET,            // int
    POSITION_X,             // int
    POSITION_Y,             // int
    POSITION_ANGLE,         // float
    ICON_TYPE,              // int (enum)
    START_ANGLE,            // int
    END_ANGLE,              // int
    FILE_PATH,              // string
    IMG_WIDTH,              // int
    IMG_HEIGHT,             // int
    PIVOT_X,                // int
    PIVOT_Y,                // int
};

class WidgetProperty {
private:
    static constexpr const std::array WidgetPropertyTypeNames = {
        "NONE"sv,
        "IS_VISIBLE"sv,
        "IS_ACTIVE"sv,
        "IS_SMOOTHED"sv,
        "MIN_VALUE"sv,
        "MAX_VALUE"sv,
        "SENSOR_ID"sv,
        "CHART_POINT_COUNT"sv,
        "CHART_TYPE"sv,
        "UI_EVENT_TYPE"sv,
        "LABEL"sv,
        "VALUE_PRECISION"sv,
        "EDGE_OFFSET"sv,
        "POSITION_X"sv,
        "POSITION_Y"sv,
        "POSITION_ANGLE"sv,
        "ICON_TYPE"sv,
        "START_ANGLE"sv,
        "END_ANGLE"sv,
        "FILE_PATH"sv,
        "IMG_WIDTH"sv,
        "IMG_HEIGHT"sv,
        "PIVOT_X"sv,
        "PIVOT_Y"sv,
    };

public:
    static const char* GetTypeName(WidgetPropertyType type) {
        int index = static_cast<int>(type);
        if(WidgetPropertyTypeNames.size() <= index)
            throw std::runtime_error("Invalid WidgetPropertyType name conversion.");

        return WidgetPropertyTypeNames[index].data();
    }

    static WidgetPropertyType GetType(const std::string& name) {
        for(size_t i = 0; i < size(WidgetPropertyTypeNames); ++i)
            if(WidgetPropertyTypeNames[i] == name)
                return static_cast<WidgetPropertyType>(i);

        throw std::runtime_error("Invalid widget property type.");
    }
};

} // namespace eerie_leap::domain::ui_domain::models
