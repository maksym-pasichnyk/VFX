#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct FlexibleFrame : View {
private:
    sp<View> content;
    std::optional<float_t> minWidth;
    std::optional<float_t> idealWidth;
    std::optional<float_t> maxWidth;
    std::optional<float_t> minHeight;
    std::optional<float_t> idealHeight;
    std::optional<float_t> maxHeight;
    Alignment alignment;

public:
    explicit FlexibleFrame(sp<View> content, std::optional<float_t> minWidth, std::optional<float_t> idealWidth, std::optional<float_t> maxWidth, std::optional<float_t> minHeight, std::optional<float_t> idealHeight, std::optional<float_t> maxHeight, Alignment alignment)
        : content(std::move(content))
        , minWidth(minWidth)
        , idealWidth(idealWidth)
        , maxWidth(maxWidth)
        , minHeight(minHeight)
        , idealHeight(idealHeight)
        , maxHeight(maxHeight)
        , alignment(alignment) {}

    auto _size(const ProposedSize& p) -> Size override {
        auto proposed = ProposedSize(p.width ?: idealWidth, p.height ?: idealHeight).orDefault(10.0F, 10.0F);
        if (minWidth.has_value()) {
            proposed.width = std::max(*minWidth, proposed.width);
        }
        if (maxWidth.has_value()) {
            proposed.width = std::min(*maxWidth, proposed.width);
        }
        if (minHeight.has_value()) {
            proposed.height = std::max(*minHeight, proposed.height);
        }
        if (maxHeight.has_value()) {
            proposed.height = std::min(*maxHeight, proposed.height);
        }

        auto size = content->_size(ProposedSize(proposed));
        if (minWidth.has_value()) {
            size.width = std::max(*minWidth, std::min(size.width, proposed.width));
        }
        if (maxWidth.has_value()) {
            size.width = std::min(*maxWidth, std::max(size.width, proposed.width));
        }
        if (minHeight.has_value()) {
            size.height = std::max(*minHeight, std::min(size.height, proposed.height));
        }
        if (maxHeight.has_value()) {
            size.height = std::min(*maxHeight, std::max(size.height, proposed.height));
        }
        return size;
    }

    void _draw(const sp<UIContext>& context, const Size& size) override {
        auto childSize = content->_size(ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        content->_draw(context, childSize);
        context->restoreState();
    }
};

inline auto View::frame(std::optional<float_t> minWidth, std::optional<float_t> idealWidth, std::optional<float_t> maxWidth, std::optional<float_t> minHeight, std::optional<float_t> idealHeight, std::optional<float_t> maxHeight, Alignment alignment) {
    return sp<FlexibleFrame>::of(RetainPtr(this), minWidth, idealWidth, maxWidth, minHeight, idealHeight, maxHeight, alignment);
}