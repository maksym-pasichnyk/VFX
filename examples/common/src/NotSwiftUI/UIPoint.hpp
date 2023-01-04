#pragma once

#include <cmath>

struct UIPoint {
    float_t x = 0.0F;
    float_t y = 0.0F;

    explicit constexpr UIPoint() = default;
    explicit constexpr UIPoint(float_t x, float_t y) : x(x), y(y) {}
};

inline constexpr auto operator+(const UIPoint& lhs, const UIPoint& rhs) -> UIPoint {
    return UIPoint(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline constexpr auto operator-(const UIPoint& lhs, const UIPoint& rhs) -> UIPoint {
    return UIPoint(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline constexpr auto operator*(const UIPoint& lhs, float_t rhs) -> UIPoint {
    return UIPoint(lhs.x * rhs, lhs.y * rhs);
}

inline constexpr auto operator/(const UIPoint& lhs, float_t rhs) -> UIPoint {
    return UIPoint(lhs.x / rhs, lhs.y / rhs);
}