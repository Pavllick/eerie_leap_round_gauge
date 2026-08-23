#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "views/renderable_base.h"
#include "views/screens/i_screen.h"

namespace eerie_leap::views {

using eerie_leap::views::screens::IScreen;

// A set of screens rendered on top of each other and shown/hidden as a unit.
class ScreenGroup : public RenderableBase {
private:
    uint32_t group_id_;
    bool is_rendered_ = false;
    bool is_activated_ = false;
    std::vector<std::shared_ptr<IScreen>> screens_;

    void ApplyScreenOrder();

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

public:
    ScreenGroup(uint32_t group_id, std::shared_ptr<Frame> parent);

    uint32_t GetGroupId() const;

    void AddScreen(std::shared_ptr<IScreen> screen);
    std::shared_ptr<IScreen> GetScreen(uint32_t screen_id) const;
    bool IsEmpty() const;

    int EnsureRendered();
    void Activate();
    void Deactivate();

    bool IsActivated() const;
};

} // namespace eerie_leap::views
