#pragma once

#include <cstdint>

namespace eerie_leap::views::widgets::configuration {

enum class WidgetCategory : uint32_t {
    None = 0,
    Indicator = 1 << 16,
    Control = 2 << 16
};

enum class WidgetType : uint32_t {
    None = 0,

    IndicatorArcFill = static_cast<uint32_t>(WidgetCategory::Indicator) | 1,
    IndicatorDigital = static_cast<uint32_t>(WidgetCategory::Indicator) | 2,
    IndicatorHorizontalChart = static_cast<uint32_t>(WidgetCategory::Indicator) | 3
};

class WidgetTypeHelpers {
public:
    static bool IsCategory(WidgetType widget_type, WidgetCategory category) {
        return (static_cast<uint32_t>(widget_type) & static_cast<uint32_t>(category)) != 0;
    }
};

} // namespace eerie_leap::views::widgets::configuration
