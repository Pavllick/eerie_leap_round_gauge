#pragma once

#include <memory>

#include <lvgl.h>

namespace eerie_leap::views::utilitites {

class Frame {
private:
    lv_obj_t* lv_object_;
    std::shared_ptr<Frame> child_;

    Frame();

    static void ValidateFrame(lv_obj_t* frame);

public:
    static Frame Create(lv_obj_t* parent);
    static Frame CreateWrapped(lv_obj_t* parent = nullptr);

    Frame Build();
    Frame Invalidate();
    Frame CleanStyles();
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
    lv_obj_t* GetParent();
    void SetChild(std::shared_ptr<Frame> child);
    std::shared_ptr<Frame> GetChild();
};

} // namespace eerie_leap::views::utilitites
