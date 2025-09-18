#pragma once

#include <memory>

#include <lvgl.h>

namespace eerie_leap::views::widgets::utilitites {

class Frame {
private:
    lv_obj_t* frame_;

    static lv_obj_t* Create(lv_obj_t* parent);
    static void ValidateFrame(lv_obj_t* frame);

public:
    Frame();

    Frame Build(lv_obj_t* parent = nullptr);
    Frame AddObject(std::shared_ptr<Frame> frame);
    Frame SetWidth(int32_t width, bool is_px);
    Frame SetHeight(int32_t height, bool is_px);
    Frame SetXOffset(int32_t offset, bool is_px);
    Frame SetYOffset(int32_t offset, bool is_px);
    Frame SetPaddingLeft(int32_t padding);
    Frame SetPaddingRight(int32_t padding);
    Frame SetPaddingTop(int32_t padding);
    Frame SetPaddingBottom(int32_t padding);
    Frame AlignBottom();
    Frame AlignTop();
    Frame AlignLeft();
    Frame AlignRight();
    Frame AlignCenter();

    lv_obj_t* GetObject();
};

} // namespace eerie_leap::views::widgets::utilitites
