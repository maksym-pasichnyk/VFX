#pragma once

#include <numeric>
#include <algorithm>

namespace gfx {
    struct ClearColor {
        float_t red   = 0.0f;
        float_t greed = 0.0f;
        float_t blue  = 0.0f;
        float_t alpha = 0.0f;

        static auto rgba(float_t r, float_t g, float_t b, float_t a) -> ClearColor {
            return { r, g, b, a };
        }

        static auto rgba32(int32_t r, int32_t g, int32_t b, int32_t a) -> ClearColor {
            return {
                std::clamp(float_t(r) / 255.f, 0.f, 1.f),
                std::clamp(float_t(g) / 255.f, 0.f, 1.f),
                std::clamp(float_t(b) / 255.f, 0.f, 1.f),
                std::clamp(float_t(a) / 255.f, 0.f, 1.f)
            };
        }
    };
}