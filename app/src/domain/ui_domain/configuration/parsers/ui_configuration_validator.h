#pragma once

#include "domain/ui_domain/models/ui_configuration.h"

namespace eerie_leap::domain::ui_domain::configuration::parsers {

using eerie_leap::domain::ui_domain::models::UiConfiguration;
using eerie_leap::domain::ui_domain::models::ScreenConfiguration;

class UiConfigurationValidator {
private:
    static void ValidateScreenCount(const UiConfiguration& configuration);
    static void ValidateScreenId(const UiConfiguration& configuration);
    static void ValidateScreenType(const UiConfiguration& configuration);
    static void ValidateScreenGrid(const UiConfiguration& configuration);
    static void ValidateActiveScreenGroupId(const UiConfiguration& configuration);

    static void ValidateScreens(const UiConfiguration& configuration);

    static void ValidateWidgets(const ScreenConfiguration& screen_configuration);
    static void ValidateWidgetId(const ScreenConfiguration& screen_configuration);
    static void ValidateWidgetType(const ScreenConfiguration& screen_configuration);
    static void ValidateWidgetSize(const ScreenConfiguration& screen_configuration);
    static void ValidateWidgetPosition(const ScreenConfiguration& screen_configuration);
    static void ValidateWidgetProperties(const ScreenConfiguration& screen_configuration);

public:
    static void Validate(const UiConfiguration& configuration);
};

} // namespace eerie_leap::domain::ui_domain::configuration::parsers
