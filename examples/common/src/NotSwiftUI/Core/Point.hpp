#pragma once

#include <cmath>

struct Point {
    float_t x;
    float_t y;

    static constexpr auto zero() -> Point {
        return Point{0, 0};
    }
};

inline constexpr auto operator+(const Point& lhs, const Point& rhs) -> Point {
    return Point{lhs.x + rhs.x, lhs.y + rhs.y};
}

inline constexpr auto operator-(const Point& lhs, const Point& rhs) -> Point {
    return Point{lhs.x - rhs.x, lhs.y - rhs.y};
}

inline constexpr auto operator*(const Point& lhs, float_t rhs) -> Point {
    return Point{lhs.x * rhs, lhs.y * rhs};
}

inline constexpr auto operator/(const Point& lhs, float_t rhs) -> Point {
    return Point{lhs.x / rhs, lhs.y / rhs};
}

inline constexpr auto operator+=(Point& lhs, const Point& rhs) -> Point& {
    return lhs = lhs + rhs;
}

inline constexpr auto operator-=(Point& lhs, const Point& rhs) -> Point& {
    return lhs = lhs - rhs;
}

inline constexpr auto operator*=(Point& lhs, float_t rhs) -> Point& {
    return lhs = lhs * rhs;
}

inline constexpr auto operator/=(Point& lhs, float_t rhs) -> Point& {
    return lhs = lhs / rhs;
}