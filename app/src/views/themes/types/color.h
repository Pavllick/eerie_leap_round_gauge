#pragma once

#include <cstdint>

#include <lvgl.h>

namespace eerie_leap::views::themes::types {

struct Color {
private:
    lv_color_t lv_color_;

public:
    const uint8_t r;
    const uint8_t g;
    const uint8_t b;
    const uint8_t a;

    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a),
        lv_color_(lv_color_make(r, g, b)) { }

    constexpr Color(uint32_t rgb, uint8_t a = 255)
        : r(rgb >> 16), g(rgb >> 8), b(rgb), a(a),
        lv_color_(lv_color_hex(rgb)) { }

    [[nodiscard]] lv_color_t ToLvColor() const {
        return lv_color_;
    }

    [[nodiscard]] lv_opa_t ToLvOpa() const {
        return a;
    }
};

} // namespace eerie_leap::views::themes::types
