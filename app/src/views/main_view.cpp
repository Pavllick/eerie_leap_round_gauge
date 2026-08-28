#include <algorithm>
#include <cerrno>
#include <string>
#include <utility>

#include <zephyr/logging/log.h>
#include <lvgl.h>

#include "domain/ui_domain/lvgl_lock.h"

#include "views/themes/theme_manager.h"

#include "main_view.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::themes;

using eerie_leap::domain::ui_domain::ScopedLvglLock;

LOG_MODULE_REGISTER(main_view_logger);

MainView::MainView() {
    container_ = std::make_shared<Frame>(Frame::CreateWrapped()
        .SetWidth(100, false)
        .SetHeight(100, false)
        .Build());
}

std::shared_ptr<Frame> MainView::GetContainer() const {
    return container_;
}

void MainView::SetRenderDispatcher(RenderDispatcher dispatcher) {
    render_dispatcher_ = std::move(dispatcher);
}

std::shared_ptr<ScreenGroup> MainView::GetOrCreateGroup(uint32_t group_id) {
    auto it = groups_.find(group_id);
    if(it != groups_.end())
        return it->second;

    auto group = std::make_shared<ScreenGroup>(group_id, container_);
    groups_.emplace(group_id, group);

    return group;
}

void MainView::AddScreen(std::shared_ptr<IScreen> screen) {
    if(screen == nullptr)
        return;

    GetOrCreateGroup(screen->GetGroupId())->AddScreen(std::move(screen));
}

std::shared_ptr<Frame> MainView::GetGroupContainer(uint32_t group_id) {
    return GetOrCreateGroup(group_id)->GetContainer();
}

void MainView::PruneEmptyGroups() {
    // GetGroupContainer() creates a group up front, so a screen that failed to
    // configure can leave behind a blank but navigable group.
    for(auto it = groups_.begin(); it != groups_.end();) {
        if(it->second->IsEmpty()) {
            LOG_WRN("Dropping screen group %u, it has no screens.", it->first);
            it = groups_.erase(it);
        } else {
            ++it;
        }
    }
}

int MainView::SetActiveGroup(uint32_t group_id) {
    auto it = groups_.find(group_id);
    if(it == groups_.end()) {
        LOG_WRN("Screen group %u is not configured.", group_id);
        return -ENOENT;
    }

    // Before the view itself is rendered only record the selection; DoRender activates it.
    if(!IsReady()) {
        active_group_id_ = group_id;
        return 0;
    }

    if(active_group_id_ == group_id)
        return 0;

    std::shared_ptr<ScreenGroup> previous_group = nullptr;
    if(active_group_id_.has_value()) {
        auto previous = groups_.find(*active_group_id_);
        if(previous != groups_.end())
            previous_group = previous->second;
    }

    if(previous_group != nullptr)
        previous_group->Deactivate();

    auto& group = it->second;

    // Recorded now, applied by the group as soon as it is rendered.
    group->Activate();
    active_group_id_ = group_id;

    if(group->IsRendered())
        return 0;

    if(render_dispatcher_ != nullptr) {
        // Capturing `this` is safe because the dispatcher's work queue is drained
        // and stopped before this view is destroyed.
        auto deferred_group = group;
        render_dispatcher_([this, deferred_group, previous_group] {
            ScopedLvglLock lvgl_guard;

            RenderGroupOrRollBack(deferred_group, previous_group);
        });

        return 0;
    }

    return RenderGroupOrRollBack(group, previous_group);
}

int MainView::RenderGroupOrRollBack(
    const std::shared_ptr<ScreenGroup>& group,
    const std::shared_ptr<ScreenGroup>& previous_group) {

    int res = group->EnsureRendered();
    if(res == 0)
        return 0;

    LOG_ERR("Failed to render screen group %u. Error: %d.", group->GetGroupId(), res);

    // A later switch already moved on; that one owns the active state now.
    if(active_group_id_ != group->GetGroupId())
        return res;

    group->Deactivate();

    active_group_id_ = previous_group != nullptr
        ? std::optional<uint32_t>(previous_group->GetGroupId())
        : std::nullopt;

    if(previous_group != nullptr)
        previous_group->Activate();

    return res;
}

std::optional<uint32_t> MainView::GetActiveGroupId() const {
    return active_group_id_;
}

std::vector<uint32_t> MainView::GetGroupIds() const {
    std::vector<uint32_t> group_ids;
    group_ids.reserve(groups_.size());

    for(const auto& [group_id, group] : groups_)
        group_ids.push_back(group_id);

    std::sort(group_ids.begin(), group_ids.end());

    return group_ids;
}

std::shared_ptr<IScreen> MainView::GetScreen(uint32_t screen_id) const {
    for(const auto& [group_id, group] : groups_) {
        auto screen = group->GetScreen(screen_id);
        if(screen != nullptr)
            return screen;
    }

    return nullptr;
}

static void RenderCenterCrossHelperGuides(lv_obj_t* screen) {
    lv_obj_t * panel1 = lv_obj_create(screen);
    lv_obj_set_width(panel1, 2);
    lv_obj_set_height(panel1, 466);
    lv_obj_set_align(panel1, LV_ALIGN_CENTER);
    lv_obj_remove_flag(panel1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(
        panel1,
        ThemeManager::GetInstance().GetCurrentTheme().GetPrimaryColor().ToLvColor(),
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * panel2 = lv_obj_create(screen);
    lv_obj_set_width(panel2, 466);
    lv_obj_set_height(panel2, 2);
    lv_obj_set_align(panel2, LV_ALIGN_CENTER);
    lv_obj_remove_flag(panel2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_color(
        panel2,
        ThemeManager::GetInstance().GetCurrentTheme().GetPrimaryColor().ToLvColor(),
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

int MainView::DoRender() {
    if(!active_group_id_.has_value()) {
        LOG_ERR("No active screen group, nothing to render.");
        return -ENOENT;
    }

    auto it = groups_.find(*active_group_id_);
    if(it == groups_.end()) {
        LOG_ERR("Active screen group %u is gone.", *active_group_id_);
        return -ENOENT;
    }

    int res = it->second->EnsureRendered();
    if(res != 0)
        return res;

    it->second->Activate();

    // RenderCenterCrossHelperGuides(container_->GetObject());

    return 0;
}

int MainView::ApplyTheme(const ITheme& theme) {
    lv_obj_set_style_bg_color(container_->GetObject(), theme.GetBackgroundColor().ToLvColor(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(container_->GetObject(), theme.GetBackgroundColor().ToLvOpa(), LV_PART_MAIN | LV_STATE_DEFAULT);

    return 0;
}

} // namespace eerie_leap::views
