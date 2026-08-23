#include <algorithm>
#include <cerrno>
#include <utility>

#include <zephyr/logging/log.h>

#include "domain/ui_domain/event_bus/ui_event_bus.h"

#include "navigation_service.h"

namespace eerie_leap::domain::ui_domain::services {

using namespace eerie_leap::domain::ui_domain::event_bus;

using eerie_leap::domain::ui_domain::models::NavigationAction;

LOG_MODULE_REGISTER(navigation_service_logger);

namespace {

void PublishNavigation(NavigationAction action, UiPayloadType target_type, uint32_t target_id) {
    UiEventPayload payload;
    payload[UiPayloadType::NavigationAction] = static_cast<uint32_t>(action);
    payload[target_type] = target_id;

    UiEventBus::GetInstance().PublishAsync({
        .type = UiEventType::NavigationChanged,
        .payload = payload
    });
}

} // namespace

NavigationService::NavigationService() {
    k_mutex_init(&lock_);
}

void NavigationService::SetGroupIds(std::vector<uint32_t> group_ids) {
    k_mutex_lock(&lock_, K_FOREVER);

    group_ids_ = std::move(group_ids);
    std::sort(group_ids_.begin(), group_ids_.end());

    k_mutex_unlock(&lock_);
}

void NavigationService::SetActiveGroupId(uint32_t group_id) {
    k_mutex_lock(&lock_, K_FOREVER);

    active_group_id_ = group_id;

    k_mutex_unlock(&lock_);
}

void NavigationService::PushHistory(uint32_t group_id) {
    if(history_count_ == k_history_depth) {
        std::rotate(history_.begin(), history_.begin() + 1, history_.end());
        --history_count_;
    }

    history_[history_count_++] = group_id;
}

std::optional<uint32_t> NavigationService::PeekHistory() const {
    if(history_count_ == 0)
        return std::nullopt;

    return history_[history_count_ - 1];
}

void NavigationService::PopHistory() {
    if(history_count_ > 0)
        --history_count_;
}

std::optional<size_t> NavigationService::GetActiveIndex() const {
    if(!active_group_id_.has_value())
        return std::nullopt;

    auto it = std::find(group_ids_.begin(), group_ids_.end(), *active_group_id_);
    if(it == group_ids_.end())
        return std::nullopt;

    return static_cast<size_t>(std::distance(group_ids_.begin(), it));
}

int NavigationService::RequestGroup(uint32_t group_id, bool record_history, std::optional<uint32_t>& group_to_publish) {
    if(active_overlay_screen_id_.has_value())
        return -EBUSY;

    if(std::find(group_ids_.begin(), group_ids_.end(), group_id) == group_ids_.end()) {
        LOG_WRN("Screen group %u is not available.", group_id);
        return -ENOENT;
    }

    if(active_group_id_ == group_id)
        return 0;

    if(record_history && active_group_id_.has_value())
        PushHistory(*active_group_id_);

    // Advance intent now; the view reconciles it via SetActiveGroupId once applied.
    // Without this, requests issued back to back would all target the same group.
    active_group_id_ = group_id;
    group_to_publish = group_id;

    return 0;
}

void NavigationService::PublishGroup(const std::optional<uint32_t>& group_to_publish) {
    if(group_to_publish.has_value())
        PublishNavigation(NavigationAction::ShowGroup, UiPayloadType::TargetGroupId, *group_to_publish);
}

int NavigationService::GoToGroup(uint32_t group_id) {
    std::optional<uint32_t> group_to_publish;

    k_mutex_lock(&lock_, K_FOREVER);
    int res = RequestGroup(group_id, true, group_to_publish);
    k_mutex_unlock(&lock_);

    PublishGroup(group_to_publish);

    return res;
}

int NavigationService::Next() {
    std::optional<uint32_t> group_to_publish;

    k_mutex_lock(&lock_, K_FOREVER);

    int res = -ENOENT;
    if(auto index = GetActiveIndex())
        res = RequestGroup(group_ids_[(*index + 1) % group_ids_.size()], true, group_to_publish);

    k_mutex_unlock(&lock_);

    PublishGroup(group_to_publish);

    return res;
}

int NavigationService::Previous() {
    std::optional<uint32_t> group_to_publish;

    k_mutex_lock(&lock_, K_FOREVER);

    int res = -ENOENT;
    if(auto index = GetActiveIndex())
        res = RequestGroup(group_ids_[(*index + group_ids_.size() - 1) % group_ids_.size()], true, group_to_publish);

    k_mutex_unlock(&lock_);

    PublishGroup(group_to_publish);

    return res;
}

int NavigationService::Back() {
    std::optional<uint32_t> group_to_publish;

    k_mutex_lock(&lock_, K_FOREVER);

    int res = -ENOENT;
    if(auto group_id = PeekHistory()) {
        res = RequestGroup(*group_id, false, group_to_publish);

        // Only consume the entry once the request was accepted.
        if(res == 0)
            PopHistory();
    }

    k_mutex_unlock(&lock_);

    PublishGroup(group_to_publish);

    return res;
}

int NavigationService::ShowOverlay(uint32_t screen_id) {
    k_mutex_lock(&lock_, K_FOREVER);
    active_overlay_screen_id_ = screen_id;
    k_mutex_unlock(&lock_);

    PublishNavigation(NavigationAction::ShowOverlay, UiPayloadType::TargetScreenId, screen_id);

    return 0;
}

int NavigationService::CloseOverlay() {
    k_mutex_lock(&lock_, K_FOREVER);
    auto closed_screen_id = active_overlay_screen_id_;
    active_overlay_screen_id_.reset();
    k_mutex_unlock(&lock_);

    if(!closed_screen_id.has_value())
        return -ENOENT;

    PublishNavigation(NavigationAction::CloseOverlay, UiPayloadType::TargetScreenId, *closed_screen_id);

    return 0;
}

int NavigationService::Handle(NavigationIntent intent, uint32_t argument) {
    switch(intent) {
        case NavigationIntent::NextGroup:
            return Next();
        case NavigationIntent::PreviousGroup:
            return Previous();
        case NavigationIntent::GoToGroup:
            return GoToGroup(argument);
        case NavigationIntent::Back:
            return Back();
        case NavigationIntent::ShowOverlay:
            return ShowOverlay(argument);
        case NavigationIntent::CloseOverlay:
            return CloseOverlay();
        case NavigationIntent::None:
        default:
            return -EINVAL;
    }
}

std::optional<uint32_t> NavigationService::GetActiveGroupId() const {
    k_mutex_lock(&lock_, K_FOREVER);
    auto active_group_id = active_group_id_;
    k_mutex_unlock(&lock_);

    return active_group_id;
}

bool NavigationService::IsOverlayActive() const {
    k_mutex_lock(&lock_, K_FOREVER);
    bool is_active = active_overlay_screen_id_.has_value();
    k_mutex_unlock(&lock_);

    return is_active;
}

} // namespace eerie_leap::domain::ui_domain::services
