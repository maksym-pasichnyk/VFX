#pragma once

#include <cmath>

struct Size {
    float_t width = 0.0F;
    float_t height = 0.0F;

    static constexpr auto zero() -> Size {
        return Size{0, 0};
    }
};

inline constexpr auto operator*(const Size& lhs, float_t rhs) -> Size {
    return Size{lhs.width * rhs, lhs.height * rhs};
}

inline constexpr auto operator/(const Size& lhs, float_t rhs) -> Size {
    return Size{lhs.width / rhs, lhs.height / rhs};
}