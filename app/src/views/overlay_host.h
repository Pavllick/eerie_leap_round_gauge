#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <lvgl.h>

#include "domain/ui_domain/services/navigation_service.h"

#include "views/renderable_base.h"
#include "views/screens/i_screen.h"

namespace eerie_leap::views {

using eerie_leap::domain::ui_domain::services::NavigationService;
using eerie_leap::views::screens::IScreen;

// How one pushed overlay behaves while it is on top.
struct OverlayOptions {
    // A scrim under the overlay swallows everything aimed at the screen groups below it.
    bool is_modal = true;

    // Tapping the scrim - that is, outside the overlay itself - closes it.
    bool close_on_scrim_tap = true;

    // 0 leaves the overlay up until something closes it.
    uint32_t auto_close_ms = 0;
};

// Hosts transient screens on lv_layer_top(), above every screen group regardless
// of the z-index they were given.
//
// Push()/Pop()/DismissAll() expect the caller to already hold the LVGL lock, like
// MainView and ScreenGroup; in the app they run on the event bus worker inside
// UiController's ScopedLvglLock. The destructor takes the lock itself because it
// is not reached from that path.
//
// Push() takes ownership of the screen. Pop() drops it, and the Frames it was
// built from delete its LVGL objects with it, so a caller that keeps its own
// reference keeps the overlay on screen.
//
// Render() the host once after construction, like MainView: a scrim is styled
// from the theme it was created under, and only a rendered host is told when
// that theme changes.
class OverlayHost : public RenderableBase {
private:
    // Deep enough for a dialog raised from a popup; anything more is a runaway publisher.
    static constexpr size_t k_max_depth = 4;
    static constexpr lv_opa_t scrim_opacity_ = LV_OPA_60;

    // One pushed overlay. Everything it owns goes away together when it does.
    struct Entry {
        Entry() = default;
        ~Entry();

        Entry(const Entry&) = delete;
        Entry& operator=(const Entry&) = delete;

        std::shared_ptr<Frame> scrim;
        lv_timer_t* auto_close_timer = nullptr;
        bool is_activated = false;

        // Declared last so it is destroyed first, before the scrim it sits on.
        std::shared_ptr<IScreen> screen;
    };

    std::shared_ptr<NavigationService> navigation_service_;
    std::vector<std::unique_ptr<Entry>> entries_;

    static void ScrimClickedCb(lv_event_t* e);
    static void AutoCloseCb(lv_timer_t* timer);

    std::shared_ptr<Frame> CreateScrim(bool close_on_tap);
    static void StyleScrim(lv_obj_t* scrim, const ITheme& theme);

    int Mount(Entry& entry, const OverlayOptions& options);
    void UpdateVisibility();

    void OnScrimClicked(const lv_obj_t* scrim);
    void OnAutoCloseElapsed(const lv_timer_t* timer);
    void RequestClose();

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

public:
    explicit OverlayHost(std::shared_ptr<NavigationService> navigation_service);
    ~OverlayHost() override;

    OverlayHost(const OverlayHost&) = delete;
    OverlayHost& operator=(const OverlayHost&) = delete;

    // Screens are parented to GetContainer() at construction, before they are pushed.
    int Push(std::shared_ptr<IScreen> screen, const OverlayOptions& options = {});
    int Pop();

    // Tears down every overlay without telling NavigationService; the caller owns
    // that state.
    void DismissAll();

    bool IsEmpty() const;
    size_t GetDepth() const;
    std::shared_ptr<IScreen> GetTopScreen() const;
};

} // namespace eerie_leap::views
