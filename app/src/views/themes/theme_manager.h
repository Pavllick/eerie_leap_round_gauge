#pragma once

#include <memory>
#include <vector>

#include "views/themes/i_theme.h"
#include "views/themes/i_theme_observer.h"

namespace eerie_leap::views::themes {

class ThemeManager {
private:
    std::shared_ptr<ITheme> current_theme_;
    std::vector<std::weak_ptr<IThemeObserver>> observers_;

    ThemeManager();
    ~ThemeManager() = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void NotifyObservers();

public:
    static ThemeManager& GetInstance();

    void SetTheme(std::shared_ptr<ITheme> theme);
    const std::shared_ptr<ITheme>& GetCurrentTheme() const;

    // Observer pattern for theme changes
    void RegisterObserver(std::shared_ptr<IThemeObserver> observer);
    void UnregisterObserver(IThemeObserver* observer);

    // Convenience accessors
    Color GetPrimaryColor() const { return current_theme_->GetPrimaryColor(); }
    Color GetBackgroundColor() const { return current_theme_->GetBackgroundColor(); }
    Color GetSurfaceColor() const { return current_theme_->GetSurfaceColor(); }
    Color GetTextColor() const { return current_theme_->GetTextColor(); }
    Color GetTextSecondaryColor() const { return current_theme_->GetTextSecondaryColor(); }
    Color GetAccentColor() const { return current_theme_->GetAccentColor(); }
    // Spacing GetDefaultSpacing() const { return current_theme_->GetDefaultSpacing(); }
    // uint8_t GetBorderRadius() const { return current_theme_->GetBorderRadius(); }
};

} // namespace eerie_leap::views::themes
