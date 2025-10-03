#pragma once

#include <memory>
#include <zephyr/kernel.h>

#include "views/utilitites/frame.h"
#include "views/i_renderable.h"
#include "views/themes/theme_manager.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::utilitites;
using namespace eerie_leap::views::themes;

class RenderableBase : public virtual IRenderable {
protected:
    std::shared_ptr<Frame> container_;
    bool is_ready_ = false;

    void OnThemeChanged() override {
        if (container_->GetObject() != nullptr && is_ready_) {
            ApplyTheme();
            container_->Invalidate();
        }
    }

public:
    RenderableBase() {
        ThemeManager::GetInstance().RegisterObserver(this);
    }

    virtual ~RenderableBase() {
        ThemeManager::GetInstance().UnregisterObserver(this);
        is_ready_= false;
    }

    std::shared_ptr<Frame> GetContainer() const override {
        return container_;
    }

    int Render() override {
        int res = DoRender();
        if(res == 0)
            res = ApplyTheme();

        container_->Invalidate();

        is_ready_ = res == 0;

        return res;
    }

    bool IsReady() const override {
        return is_ready_;
    }
};

} // namespace eerie_leap
