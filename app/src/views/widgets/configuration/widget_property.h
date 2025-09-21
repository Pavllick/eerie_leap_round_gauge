#pragma once

#include <cstdint>
#include <array>
#include <stdexcept>

namespace eerie_leap::views::widgets::configuration {

using namespace std::string_view_literals;

enum class WidgetPropertyType : std::uint16_t {
    MIN_VALUE,
    MAX_VALUE,
    SENSOR_ID,
    CHART_POINT_COUNT,
    CHART_TYPE
};

class WidgetProperty {
private:
    static constexpr const std::array WidgetPropertyTypeNames = {
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

} // namespace eerie_leap::views::widgets::configuration
