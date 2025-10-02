#include <stdexcept>

#include "frame.h"

namespace eerie_leap::views::utilitites {

Frame::Frame() : lv_object_(nullptr) { }

Frame Frame::Create(lv_obj_t* object) {
    Frame frame;
    frame.lv_object_ = object;

    return frame;
}

Frame Frame::CreateWrapped(lv_obj_t* object) {
    lv_obj_t* frame = lv_obj_create(object == nullptr ? lv_screen_active() : object);
    lv_obj_remove_style_all(frame);

    return Frame::Create(frame);
}

Frame Frame::Build() {
    return *this;
}

void Frame::ValidateFrame(lv_obj_t* frame) {
    if(!frame)
        throw std::runtime_error("Frame not created");
}

Frame Frame::Invalidate() {
    ValidateFrame(lv_object_);

    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::CleanStyles() {
    ValidateFrame(lv_object_);

    lv_obj_remove_style_all(lv_object_);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetWidth(int32_t width, bool is_px) {
    ValidateFrame(lv_object_);

    lv_obj_set_width(lv_object_, is_px ? width : lv_pct(width));
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetHeight(int32_t height, bool is_px) {
    ValidateFrame(lv_object_);

    lv_obj_set_height(lv_object_, is_px ? height : lv_pct(height));
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetXOffset(int32_t offset, bool is_px) {
    ValidateFrame(lv_object_);

    lv_obj_set_x(lv_object_, is_px ? offset : lv_pct(offset));
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetYOffset(int32_t offset, bool is_px) {
    ValidateFrame(lv_object_);

    lv_obj_set_y(lv_object_, is_px ? offset : lv_pct(offset));
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetPaddingLeft(int32_t padding) {
    ValidateFrame(lv_object_);

    lv_obj_set_style_pad_left(lv_object_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetPaddingRight(int32_t padding) {
    ValidateFrame(lv_object_);

    lv_obj_set_style_pad_right(lv_object_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetPaddingTop(int32_t padding) {
    ValidateFrame(lv_object_);

    lv_obj_set_style_pad_top(lv_object_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::SetPaddingBottom(int32_t padding) {
    ValidateFrame(lv_object_);

    lv_obj_set_style_pad_bottom(lv_object_, padding, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::AlignBottom() {
    ValidateFrame(lv_object_);

    lv_obj_set_align(lv_object_, LV_ALIGN_BOTTOM_MID);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::AlignTop() {
    ValidateFrame(lv_object_);

    lv_obj_set_align(lv_object_, LV_ALIGN_TOP_MID);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::AlignLeft() {
    ValidateFrame(lv_object_);

    lv_obj_set_align(lv_object_, LV_ALIGN_LEFT_MID);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::AlignRight() {
    ValidateFrame(lv_object_);

    lv_obj_set_align(lv_object_, LV_ALIGN_RIGHT_MID);
    lv_obj_invalidate(lv_object_);

    return *this;
}

Frame Frame::AlignCenter() {
    ValidateFrame(lv_object_);

    lv_obj_set_align(lv_object_, LV_ALIGN_CENTER);
    lv_obj_invalidate(lv_object_);

    return *this;
}

lv_obj_t* Frame::GetObject() {
    return lv_object_;
}

void Frame::SetChild(std::shared_ptr<Frame> child) {
    child_ = std::move(child);
}

std::shared_ptr<Frame> Frame::GetChild() {
    return child_;
}

} // namespace eerie_leap::views::utilitites
