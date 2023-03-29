#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct FixedFrame : View {
private:
    sp<View> content;
    std::optional<float_t> width;
    std::optional<float_t> height;
    Alignment alignment;

public:
    explicit FixedFrame(sp<View> content, std::optional<float_t> width, std::optional<float_t> height, Alignment alignment)
        : content(std::move(content)), width(width), height(height), alignment(alignment) {}

    auto _size(const sp<UIContext> &context, const ProposedSize& proposed) -> Size override {
        auto cgSize = proposed.orMax();
        auto childSize = content->_size(context, ProposedSize(width.value_or(cgSize.width), height.value_or(cgSize.height)));
        return Size{width.value_or(childSize.width), height.value_or(childSize.height)};
    }

    void _draw(const sp<UIContext>& context, const Size& size) override {
        auto childSize = content->_size(context, ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        content->_draw(context, childSize);
        context->restoreState();
    }
};

inline auto View::frame(std::optional<float_t> width, std::optional<float_t> height, Alignment alignment) {
    return sp<FixedFrame>::of(RetainPtr(this), width, height, alignment);
}