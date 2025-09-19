#pragma once

#include <memory>
#include <vector>

#include "views/screens/configuration/screen_configuration.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::screens {

using namespace eerie_leap::views::widgets;
using namespace eerie_leap::views::screens::configuration;

class IScreen {
public:
    virtual int Render() = 0;
    virtual void Configure(ScreenConfiguration& config) = 0;
    virtual std::shared_ptr<std::vector<std::shared_ptr<IWidget>>> GetWidgets() const = 0;
    virtual ~IScreen() = default;
};

} // namespace eerie_leap::views::screens
