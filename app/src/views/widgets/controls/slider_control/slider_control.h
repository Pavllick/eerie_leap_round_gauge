#pragma once

#include <cstdint>
#include <memory>

#include <lvgl.h>

#include "views/widgets/controls/control_base.h"

namespace eerie_leap::views::widgets::controls {

class SliderControl : public ControlBase {
private:
    lv_obj_t* lv_slider_ = nullptr;
    double configured_step_ = 0;
    double min_ = 0;
    double max_ = 0;
    double step_ = 1;
    int32_t step_count_ = 1;

    int32_t ToIndex(double value) const;
    double ToValue(int32_t index) const;
    void ApplyRange();
    void SyncFromSetting();

    int DoRender() override;
    int ApplyTheme(const ITheme& theme) override;

protected:
    void OnRangeResolved() override;
    void OnControlEvent(lv_event_code_t code) override;
    void OnSettingChanged() override;

public:
    explicit SliderControl(uint32_t id, std::shared_ptr<Frame> parent, WidgetContext context);

    void Configure(std::shared_ptr<WidgetConfiguration> configuration) override;
    WidgetType GetType() const override { return WidgetType::ControlSlider; }
};

} // namespace eerie_leap::views::widgets::controls
