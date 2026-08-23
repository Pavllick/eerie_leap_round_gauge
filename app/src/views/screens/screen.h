#pragma once

#include <memory>
#include <vector>

#include "subsys/assets/assets_manager.h"
#include "domain/ui_domain/models/screen_configuration.h"

#include "views/renderable_base.h"
#include "views/widgets/i_widget.h"
#include "views/screens/i_screen.h"
#include "views/utilitites/frame.h"
#include "views/utilitites/grid_layout.h"

namespace eerie_leap::views::screens {

using eerie_leap::subsys::assets::AssetsManager;
using eerie_leap::domain::ui_domain::models::GridSettings;
using eerie_leap::views::utilitites::GridLayout;

class Screen : public RenderableBase, public IScreen {
protected:
    std::shared_ptr<AssetsManager> ui_assets_manager_;
    uint32_t id_;
    std::shared_ptr<Frame> parent_;

    std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> widgets_;
    std::shared_ptr<ScreenConfiguration> configuration_;

    void UpdateWidgetGeometry(IWidget& widget, const GridLayout& layout);
    void SetVisibility(bool is_visible);

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

public:
    Screen(
        std::shared_ptr<AssetsManager> ui_assets_manager,
        uint32_t id,
        std::shared_ptr<Frame> parent);

    void Configure(std::shared_ptr<ScreenConfiguration> configuration) override;
    std::shared_ptr<ScreenConfiguration> GetConfiguration() const override;

    uint32_t GetId() const override;
    uint32_t GetGroupId() const override;
    int32_t GetZIndex() const override;
    bool IsVisible() const override;

    void OnActivated() override;
    void OnDeactivated() override;

    std::shared_ptr<std::vector<std::unique_ptr<IWidget>>> GetWidgets() const override;
};

} // namespace eerie_leap::views::screens
