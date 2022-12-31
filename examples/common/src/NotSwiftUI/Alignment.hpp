#pragma once

#include "UISize.hpp"
#include "UIPoint.hpp"

struct VerticalAlignment {
    auto (*defaultValue)(const UISize& size) -> float_t;

    static constexpr auto top() -> VerticalAlignment {
        return {[](const UISize& size) -> float_t {
            return 0;
        }};
    }

    static constexpr auto center() -> VerticalAlignment {
        return {[](const UISize& size) -> float_t {
            return size.height * 0.5F;
        }};
    }

    static constexpr auto bottom() -> VerticalAlignment {
        return {[](const UISize& size) -> float_t {
            return size.height;
        }};
    }
};

struct HorizontalAlignment {
    auto (*defaultValue)(const UISize& size) -> float_t;

    static constexpr auto trailing() -> HorizontalAlignment {
        return {[](const UISize& size) -> float_t {
            return size.width;
        }};
    }

    static constexpr auto center() -> HorizontalAlignment {
        return {[](const UISize& size) -> float_t {
            return size.width * 0.5F;
        }};
    }

    static constexpr auto leading() -> HorizontalAlignment {
        return {[](const UISize& size) -> float_t {
            return 0;
        }};
    }
};

struct Alignment {
    HorizontalAlignment mHorizontalAlignment;
    VerticalAlignment mVerticalAlignment;

    auto point(const UISize& size) const -> UIPoint {
        return {
            mHorizontalAlignment.defaultValue(size),
            mVerticalAlignment.defaultValue(size),
        };
    }

    static constexpr auto topLeading() -> Alignment {
        return {HorizontalAlignment::leading(), VerticalAlignment::top()};
    }

    static constexpr auto bottomLeading() -> Alignment {
        return {HorizontalAlignment::leading(), VerticalAlignment::bottom()};
    }

    static constexpr auto center() -> Alignment {
        return {HorizontalAlignment::center(), VerticalAlignment::center()};
    }
};