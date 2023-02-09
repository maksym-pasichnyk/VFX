#pragma once

#include <optional>

#include "Size.hpp"

struct ProposedSize {
    std::optional<float_t> width;
    std::optional<float_t> height;

    constexpr explicit ProposedSize(std::optional<float_t> width, std::optional<float_t> height) : width(width), height(height) {}
    constexpr explicit ProposedSize(const Size& size) : width(size.width), height(size.height) {}

    [[nodiscard]] auto orMax() const -> Size {
        float_t w = width.value_or(std::numeric_limits<float_t>::max());
        float_t h = height.value_or(std::numeric_limits<float_t>::max());
        return {w, h};
    }

    [[nodiscard]] auto orDefault(float_t defaultWidth, float_t defaultHeight) const -> Size {
        float_t w = width.value_or(defaultWidth);
        float_t h = height.value_or(defaultHeight);
        return Size{w, h};
    }
};
