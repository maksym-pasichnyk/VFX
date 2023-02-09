#pragma once

#include <cmath>

struct Color {
    float_t r;
    float_t g;
    float_t b;
    float_t a;

    static auto rgba32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> Color {
        return Color{
            static_cast<float_t>(r) / 255.0F,
            static_cast<float_t>(g) / 255.0F,
            static_cast<float_t>(b) / 255.0F,
            static_cast<float_t>(a) / 255.0F
        };
    }
};
