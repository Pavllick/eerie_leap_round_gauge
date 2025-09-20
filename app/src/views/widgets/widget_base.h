#pragma once

#include <memory>

#include <lvgl.h>

#include "views/widgets/utilitites/frame.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::views::widgets::utilitites;
using namespace eerie_leap::views::widgets::configuration;

class WidgetBase : public IWidget {
private:
    bool is_animation_enabled_;

protected:
    uint32_t id_;

    WidgetConfiguration configuration_;
    WidgetPosition position_px_;
    WidgetSize size_px_;

    std::shared_ptr<Frame> container_;
    lv_obj_t* lv_obj_;

public:
    WidgetBase(uint32_t id);

    uint32_t GetId() const override;
    bool IsAnimationEnabled() const override;

    void Configure(const WidgetConfiguration& config) override;
    WidgetConfiguration GetConfiguration() const override;

    WidgetPosition GetPositionPx() const override;
    void SetPositionPx(const WidgetPosition& pos) override;
    WidgetSize GetSizePx() const override;
    void SetSizePx(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets
