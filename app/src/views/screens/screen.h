#pragma once

#include <memory>
#include <vector>

#include "domain/ui_domain/models/screen_configuration.h"
#include "domain/ui_domain/assets_manager/ui_assets_manager.h"

#include "views/renderable_base.h"
#include "views/widgets/i_widget.h"
#include "views/screens/i_screen.h"
#include "views/utilitites/frame.h"

namespace eerie_leap::views::screens {

using eerie_leap::domain::ui_domain::models::GridSettings;
using eerie_leap::domain::ui_domain::assets_manager::UiAssetsManager;

class Screen : public RenderableBase, public IScreen {
protected:
    std::shared_ptr<UiAssetsManager> ui_assets_manager_;
    uint32_t id_;
    std::shared_ptr<Frame> parent_;

    std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> widgets_;
    std::shared_ptr<ScreenConfiguration> configuration_;

    void UpdateWidgetSize(IWidget& widget, GridSettings& grid_settings, int32_t screen_width, int32_t screen_height);
    void UpdateWidgetPosition(IWidget& widget, GridSettings& grid_settings);

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

public:
    Screen(
        std::shared_ptr<UiAssetsManager> ui_assets_manager,
        uint32_t id,
        std::shared_ptr<Frame> parent);

    void Configure(std::shared_ptr<ScreenConfiguration> configuration) override;
    std::shared_ptr<ScreenConfiguration> GetConfiguration() const override;

    std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> GetWidgets() const override;
};

} // namespace eerie_leap::views::screens
