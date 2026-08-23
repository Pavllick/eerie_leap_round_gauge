#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <lvgl.h>

#include "views/renderable_base.h"
#include "views/screen_group.h"
#include "views/screens/i_screen.h"

namespace eerie_leap::views {

using eerie_leap::views::screens::IScreen;

class MainView : public RenderableBase {
private:
    std::unordered_map<uint32_t, std::shared_ptr<ScreenGroup>> groups_;
    std::optional<uint32_t> active_group_id_;

    std::shared_ptr<ScreenGroup> GetOrCreateGroup(uint32_t group_id);

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

public:
    MainView();

    std::shared_ptr<Frame> GetContainer() const;

    // Screens must be parented to their group's container, so it is created up front.
    std::shared_ptr<Frame> GetGroupContainer(uint32_t group_id);

    void AddScreen(std::shared_ptr<IScreen> screen);
    void PruneEmptyGroups();

    int SetActiveGroup(uint32_t group_id);
    std::optional<uint32_t> GetActiveGroupId() const;

    std::vector<uint32_t> GetGroupIds() const;
    std::shared_ptr<IScreen> GetScreen(uint32_t screen_id) const;
};

} // namespace eerie_leap::views
