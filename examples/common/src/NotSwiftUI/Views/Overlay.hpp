#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct Overlay : View {
private:
    sp<View> content;
    sp<View> overlay;
    Alignment alignment;

public:
    explicit Overlay(sp<View> content, sp<View> overlay, Alignment alignment)
        : content(std::move(content)), overlay(std::move(overlay)), alignment(alignment) {}

    auto _size(const sp<UIContext> &context, const ProposedSize& proposed) -> Size override {
        return content->_size(context, proposed);
    }

    void _draw(const sp<UIContext>& context, const Size& size) override {
        content->_draw(context, size);

        auto childSize = overlay->_size(context, ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        overlay->_draw(context, childSize);
        context->restoreState();
    }
};

inline auto View::overlay(sp<View> overlay, Alignment alignment) {
    return sp<Overlay>::of(RetainPtr(this), std::move(overlay), alignment);
}