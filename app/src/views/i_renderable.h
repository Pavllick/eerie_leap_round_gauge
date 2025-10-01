#pragma once

#include <memory>

#include "views/utilitites/frame.h"
#include "views/themes/i_theme_observer.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::themes;
using namespace eerie_leap::views::utilitites;

class IRenderable : public IThemeObserver {
protected:
    virtual int DoRender() = 0;
    virtual int ApplyTheme() = 0;

public:
    virtual ~IRenderable() = default;

    virtual int Render() = 0;
    virtual bool IsReady() const = 0;
};

} // namespace eerie_leap
