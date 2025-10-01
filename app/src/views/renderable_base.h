#pragma once

#include <memory>

#include "views/utilitites/frame.h"
#include "views/i_renderable.h"

namespace eerie_leap::views {

using namespace eerie_leap::views::utilitites;

class RenderableBase : public virtual IRenderable {
protected:
    std::shared_ptr<Frame> container_;
    bool is_ready_ = false;

    void OnThemeChanged() override {
        if (container_->GetChild() != nullptr && is_ready_) {
            ApplyTheme();
            lv_obj_invalidate(container_->GetChild()->GetObject());
        }
    }

public:
    virtual ~RenderableBase() {
        is_ready_= false;
    }

    int Render() override {
        int res = DoRender();
        if(res == 0)
            res = ApplyTheme();

        is_ready_ = res == 0;

        return res;
    }

    bool IsReady() const override {
        return is_ready_;
    }
};

} // namespace eerie_leap
