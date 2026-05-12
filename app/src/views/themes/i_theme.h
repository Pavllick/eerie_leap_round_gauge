#pragma once

#include "views/themes/types/color.h"
#include "views/themes/types/font.h"
#include "views/assets/fonts/fonts_register.h"

namespace eerie_leap::views::themes {

using eerie_leap::views::themes::types::Color;
using eerie_leap::views::themes::types::Font;

class ITheme {
public:
    virtual ~ITheme() = default;

    // Colors
    [[nodiscard]] virtual Color GetPrimaryColor() const = 0;
    [[nodiscard]] virtual Color GetSecondaryColor() const = 0;
    [[nodiscard]] virtual Color GetInactiveColor() const = 0;
    [[nodiscard]] virtual Color GetBackgroundColor() const = 0;
    [[nodiscard]] virtual Color GetSurfaceColor() const = 0;
    [[nodiscard]] virtual Color GetAccentColor() const = 0;
    [[nodiscard]] virtual Color GetErrorColor() const = 0;

    [[nodiscard]] virtual Font GetPrimaryFont() const = 0;
    [[nodiscard]] virtual Font GetSecondaryFont() const = 0;
    [[nodiscard]] virtual Font GetPrimaryFontLarge() const = 0;
};

} // namespace eerie_leap::views::themes
