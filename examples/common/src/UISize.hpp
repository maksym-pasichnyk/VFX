#pragma once

#include <cmath>

struct UISize {
    float_t width = 0.0F;
    float_t height = 0.0F;

    explicit constexpr UISize() = default;
    explicit constexpr UISize(float_t width, float_t height) : width(width), height(height) {}
};

inline constexpr auto operator*(const UISize& lhs, float_t rhs) -> UISize {
    return UISize(lhs.width * rhs, lhs.height * rhs);
}

inline constexpr auto operator/(const UISize& lhs, float_t rhs) -> UISize {
    return UISize(lhs.width / rhs, lhs.height / rhs);
}