#pragma once

#include <cstdint>

namespace eerie_leap::domain::ui_domain::models {

enum class WidgetCategory : uint32_t {
    None = 0,
    Basic = 1 << 16,
    Indicator = 2 << 16,
    Control = 3 << 16
};

enum class WidgetType : uint32_t {
    None = 0,

    BasicIcon = static_cast<uint32_t>(WidgetCategory::Basic) | 1,
    BasicLabelIcon = static_cast<uint32_t>(WidgetCategory::Basic) | 2,
    BasicArcLabelIcon = static_cast<uint32_t>(WidgetCategory::Basic) | 3,

    IndicatorArcFill = static_cast<uint32_t>(WidgetCategory::Indicator) | 101,
    IndicatorDigital = static_cast<uint32_t>(WidgetCategory::Indicator) | 102,
    IndicatorHorizontalChart = static_cast<uint32_t>(WidgetCategory::Indicator) | 103
};

class WidgetTypeHelpers {
public:
    static bool IsCategory(WidgetType widget_type, WidgetCategory category) {
        return (static_cast<uint32_t>(widget_type) & static_cast<uint32_t>(category)) != 0;
    }
};

} // namespace eerie_leap::domain::ui_domain::models
