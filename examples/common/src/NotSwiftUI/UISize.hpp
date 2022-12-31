#pragma once

#include <cmath>

struct UISize {
    float_t width;
    float_t height;

    explicit UISize(float_t width, float_t height) : width(width), height(height) {}
};