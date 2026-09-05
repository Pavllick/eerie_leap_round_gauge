#pragma once

#include <cstdint>
#include <memory>
#include <memory_resource>
#include <optional>

#include <lvgl.h>

#include "views/widgets/controls/control_base.h"

namespace eerie_leap::views::widgets::controls {

class ButtonControl : public ControlBase {
private:
    lv_obj_t* lv_button_ = nullptr;
    lv_obj_t* lv_label_ = nullptr;

    std::pmr::string label_;
    std::optional<uint32_t> target_group_id_;

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

protected:
    void OnControlEvent(lv_event_code_t code) override;

public:
    explicit ButtonControl(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context);

protected:
    void RegisterProperties(WidgetPropertyStore& store) const override;
    void OnPropertyChanged(WidgetPropertyType type, const ConfigValue& value) override;

public:
    WidgetType GetType() const override { return WidgetType::ControlButton; }
};

} // namespace eerie_leap::views::widgets::controls
