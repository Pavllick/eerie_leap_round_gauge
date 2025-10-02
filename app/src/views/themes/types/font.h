#pragma once

#include <string>

#include <lvgl.h>

namespace eerie_leap::views::themes::types {

struct Font {
private:
    const lv_font_t* lv_font_;

public:
    const std::string name;

    constexpr Font(std::string name, const lv_font_t* font, uint32_t size)
        : lv_font_(font), name(std::move(name)) { }

    [[nodiscard]] const lv_font_t* ToLvFont() const {
        return lv_font_;
    }
};

} // namespace eerie_leap::views::themes::types
