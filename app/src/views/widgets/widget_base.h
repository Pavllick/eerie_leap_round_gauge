#pragma once

#include <memory>
#include <unordered_set>

#include <lvgl.h>

#include "views/widgets/utilitites/frame.h"
#include "views/widgets/i_widget.h"

namespace eerie_leap::views::widgets {

using namespace eerie_leap::domain::ui_domain::models;
using namespace eerie_leap::views::widgets::utilitites;

class WidgetBase : public IWidget {
protected:
    uint32_t id_;
    std::unordered_set<WidgetTag> tags_;

    WidgetConfiguration configuration_;
    WidgetPosition position_px_;
    WidgetSize size_px_;

    std::shared_ptr<Frame> container_;
    lv_obj_t* lv_obj_;

    void UpdateTags(std::vector<int> tag_values);
    int SetVisibility(bool is_visible);

public:
    WidgetBase(uint32_t id);

    uint32_t GetId() const override;
    bool HasTag(WidgetTag tag) const override;
    bool IsAnimated() const override;
    bool IsVisible() const override;

    void Configure(const WidgetConfiguration& config) override;
    WidgetConfiguration GetConfiguration() const override;
    bool UpdateProperty(const WidgetPropertyType property_type, const ConfigValue& value, bool force_update = false) override;

    WidgetPosition GetPositionPx() const override;
    void SetPositionPx(const WidgetPosition& pos) override;
    WidgetSize GetSizePx() const override;
    void SetSizePx(const WidgetSize& size) override;
};

} // namespace eerie_leap::views::widgets
