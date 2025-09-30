#pragma once

#include <memory>
#include <vector>

#include "domain/ui_domain/models/screen_configuration.h"

#include "views/widgets/i_widget.h"
#include "views/screens/i_screen.h"
#include "views/utilitites/frame.h"

namespace eerie_leap::views::screens {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::widgets;

class Screen : public IScreen, public IRenderable {
protected:
    uint32_t id_;
    std::shared_ptr<Frame> container_;

    std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> widgets_;
    ScreenConfiguration configuration_;

    void UpdateWidgetSize(std::unique_ptr<IWidget>& widget, GridSettings& grid_settings);
    void UpdateWidgetPosition(std::unique_ptr<IWidget>& widget, GridSettings& grid_settings);

    int DoRender() override;

public:
    Screen(uint32_t id);

    void Configure(const ScreenConfiguration& config) override;
    ScreenConfiguration GetConfiguration() const override;

    std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> GetWidgets() const override;
};

} // namespace eerie_leap::views::screens
