#pragma once

#include <memory>
#include <vector>

#include "domain/ui_domain/models/screen_configuration.h"
#include "views/i_renderable.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::screens {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views;
using namespace eerie_leap::views::widgets;

class IScreen : public IRenderable {
public:
    virtual void Configure(const ScreenConfiguration& config) = 0;
    virtual ScreenConfiguration GetConfiguration() const = 0;
    virtual std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> GetWidgets() const = 0;
    virtual ~IScreen() = default;
};

} // namespace eerie_leap::views::screens
