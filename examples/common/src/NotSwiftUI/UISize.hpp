#pragma once

#include <cmath>

struct UISize {
    float_t width = 0.0F;
    float_t height = 0.0F;

    explicit constexpr UISize() = default;
    explicit constexpr UISize(float_t width, float_t height) : width(width), height(height) {}
};