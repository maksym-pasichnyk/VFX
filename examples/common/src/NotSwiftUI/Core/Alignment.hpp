#pragma once

#include "Point.hpp"

struct VerticalAlignment {
    auto (*defaultValue)(const Size& size) -> float_t;

    static constexpr auto top() -> VerticalAlignment {
        return {[](const Size& size) -> float_t {
            return 0;
        }};
    }

    static constexpr auto center() -> VerticalAlignment {
        return {[](const Size& size) -> float_t {
            return size.height * 0.5F;
        }};
    }

    static constexpr auto bottom() -> VerticalAlignment {
        return {[](const Size& size) -> float_t {
            return size.height;
        }};
    }
};

struct HorizontalAlignment {
    auto (*defaultValue)(const Size& size) -> float_t;

    static constexpr auto trailing() -> HorizontalAlignment {
        return {[](const Size& size) -> float_t {
            return size.width;
        }};
    }

    static constexpr auto center() -> HorizontalAlignment {
        return {[](const Size& size) -> float_t {
            return size.width * 0.5F;
        }};
    }

    static constexpr auto leading() -> HorizontalAlignment {
        return {[](const Size& size) -> float_t {
            return 0;
        }};
    }
};

struct Alignment {
    HorizontalAlignment mHorizontalAlignment;
    VerticalAlignment mVerticalAlignment;

    explicit constexpr Alignment(HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment)
        : mHorizontalAlignment(horizontalAlignment), mVerticalAlignment(verticalAlignment) {}

    auto point(const Size& size) const -> Point {
        float_t x = mHorizontalAlignment.defaultValue(size);
        float_t y = mVerticalAlignment.defaultValue(size);
        return {x, y};
    }

    static constexpr auto topLeading() -> Alignment {
        return Alignment(HorizontalAlignment::leading(), VerticalAlignment::top());
    }

    static constexpr auto bottomLeading() -> Alignment {
        return Alignment(HorizontalAlignment::leading(), VerticalAlignment::bottom());
    }

    static constexpr auto center() -> Alignment {
        return Alignment(HorizontalAlignment::center(), VerticalAlignment::center());
    }
};