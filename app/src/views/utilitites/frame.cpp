#include <stdexcept>

#include "frame.h"

namespace eerie_leap::views::utilitites {

Frame::Frame() : frame_(nullptr) { }

lv_obj_t* Frame::Create(lv_obj_t* parent) {
    lv_obj_t* frame = lv_obj_create(parent);
    lv_obj_remove_style_all(frame);

    return frame;
}

Frame Frame::Build(lv_obj_t* parent) {
    frame_ = Create(parent == nullptr ? lv_screen_active() : parent);

    SetWidth(100, false);
    SetHeight(100, false);

    return *this;
}

void Frame::ValidateFrame(lv_obj_t* frame) {
    if(!frame)
        throw std::runtime_error("Frame not created");
}

Frame Frame::AddObject(std::shared_ptr<Frame> frame) {
    ValidateFrame(frame_);

    lv_obj_set_parent(frame->GetObject(), frame_);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetWidth(int32_t width, bool is_px) {
    ValidateFrame(frame_);

    lv_obj_set_width(frame_, is_px ? width : lv_pct(width));
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetHeight(int32_t height, bool is_px) {
    ValidateFrame(frame_);

    lv_obj_set_height(frame_, is_px ? height : lv_pct(height));
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetXOffset(int32_t offset, bool is_px) {
    ValidateFrame(frame_);

    lv_obj_set_x(frame_, is_px ? offset : lv_pct(offset));
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetYOffset(int32_t offset, bool is_px) {
    ValidateFrame(frame_);

    lv_obj_set_y(frame_, is_px ? offset : lv_pct(offset));
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetPaddingLeft(int32_t padding) {
    ValidateFrame(frame_);

    lv_obj_set_style_pad_left(frame_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetPaddingRight(int32_t padding) {
    ValidateFrame(frame_);

    lv_obj_set_style_pad_right(frame_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetPaddingTop(int32_t padding) {
    ValidateFrame(frame_);

    lv_obj_set_style_pad_top(frame_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::SetPaddingBottom(int32_t padding) {
    ValidateFrame(frame_);

    lv_obj_set_style_pad_bottom(frame_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::AlignBottom() {
    ValidateFrame(frame_);

    lv_obj_set_align(frame_, LV_ALIGN_BOTTOM_MID);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::AlignTop() {
    ValidateFrame(frame_);

    lv_obj_set_align(frame_, LV_ALIGN_TOP_MID);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::AlignLeft() {
    ValidateFrame(frame_);

    lv_obj_set_align(frame_, LV_ALIGN_LEFT_MID);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::AlignRight() {
    ValidateFrame(frame_);

    lv_obj_set_align(frame_, LV_ALIGN_RIGHT_MID);
    lv_obj_invalidate(frame_);

    return *this;
}

Frame Frame::AlignCenter() {
    ValidateFrame(frame_);

    lv_obj_set_align(frame_, LV_ALIGN_CENTER);
    lv_obj_invalidate(frame_);

    return *this;
}

lv_obj_t* Frame::GetObject() { return frame_; }

} // namespace eerie_leap::views::utilitites
