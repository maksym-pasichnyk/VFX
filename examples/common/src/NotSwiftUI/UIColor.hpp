#pragma once

#include <cmath>

struct UIColor {
    float_t r;
    float_t g;
    float_t b;
    float_t a;

    explicit UIColor(float_t r, float_t g, float_t b, float_t a) : r(r), g(g), b(b), a(a) {}
};
