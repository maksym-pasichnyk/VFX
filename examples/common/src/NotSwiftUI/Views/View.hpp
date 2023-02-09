#pragma once

#include "Object.hpp"
#include "UIContext.hpp"
#include "NotSwiftUI/Core/Size.hpp"
#include "NotSwiftUI/Core/Point.hpp"
#include "NotSwiftUI/Core/Alignment.hpp"
#include "NotSwiftUI/Core/ProposedSize.hpp"

#include <optional>

struct View : Object {
    auto frame(std::optional<float_t> width, std::optional<float_t> height, Alignment alignment = Alignment::center());
    auto frame(std::optional<float_t> minWidth, std::optional<float_t> idealWidth, std::optional<float_t> maxWidth, std::optional<float_t> minHeight, std::optional<float_t> idealHeight, std::optional<float_t> maxHeight, Alignment alignment = Alignment::center());
    auto border(const Color& color, float_t width);
    auto overlay(sp<View> overlay, Alignment alignment = Alignment::center());
    auto fixedSize(bool horizontal, bool vertical);
    auto foregroundColor(const Color& color);

    virtual auto body() -> sp<View> {
        throw std::runtime_error("FatalError");
    }

    virtual auto _size(const ProposedSize& proposed) -> Size {
        return body()->_size(proposed);
    }

    virtual void _draw(const sp<UIContext>& context, const Size& size) {
        return body()->_draw(context, size);
    }

    static auto translation(const Size& childSize, const Size& parentSize, const Alignment& alignment) -> Point {
        Point childPoint = alignment.point(childSize);
        Point parentPoint = alignment.point(parentSize);
        return parentPoint - childPoint;
    }
};
