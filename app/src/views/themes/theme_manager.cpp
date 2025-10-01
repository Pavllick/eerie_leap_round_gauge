#include <algorithm>

#include "views/themes/default_theme.h"

#include "theme_manager.h"

namespace eerie_leap::views::themes {

static DefaultTheme default_theme_;

ThemeManager::ThemeManager()
    : current_theme_(&default_theme_) {
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

void ThemeManager::RegisterObserver(std::shared_ptr<IThemeObserver> observer) {
    if(!observer)
        return;

    observers_.push_back(observer);
}

void ThemeManager::UnregisterObserver(IThemeObserver* observer) {
    if(!observer)
        return;

    observers_.erase(
        std::remove_if(observers_.begin(), observers_.end(),
            [observer](const std::weak_ptr<IThemeObserver>& w) {
                auto s = w.lock();
                return !s || s.get() == observer;
            }),
        observers_.end()
    );
}

void ThemeManager::NotifyObservers() {
    for (auto it = observers_.begin(); it != observers_.end(); ) {
        if (auto observer = it->lock()) {
            observer->OnThemeChanged();
            ++it;
        } else {
            it = observers_.erase(it);
        }
    }
}

} // namespace eerie_leap::views::themes
