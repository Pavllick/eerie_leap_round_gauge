#include <algorithm>

#include "views/themes/default_theme.h"

#include "theme_manager.h"

namespace eerie_leap::views::themes {

ThemeManager::ThemeManager()
    : current_theme_(std::make_shared<DefaultTheme>()) {
}

ThemeManager& ThemeManager::GetInstance() {
    static ThemeManager instance;
    return instance;
}

void ThemeManager::SetTheme(std::shared_ptr<ITheme> theme) {
    if (theme != nullptr && theme != current_theme_) {
        current_theme_ = theme;
        NotifyObservers();
    }
}

const std::shared_ptr<ITheme>& ThemeManager::GetCurrentTheme() const {
    return current_theme_;
}

void ThemeManager::RegisterObserver(IThemeObserver* observer) {
    if(observer != nullptr)
        observers_.push_back(observer);
}

void ThemeManager::UnregisterObserver(IThemeObserver* observer) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), observer),
        observers_.end()
    );
}

void ThemeManager::NotifyObservers() {
    for(auto* observer : observers_)
        observer->OnThemeChanged();
}

} // namespace eerie_leap::views::themes
