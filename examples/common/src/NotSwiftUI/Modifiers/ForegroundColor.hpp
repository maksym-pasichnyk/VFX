#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct ForegroundColor : View {
private:
    sp<View> content;
    Color color;

public:
    explicit ForegroundColor(sp<View> content, const Color& color)
        : content(std::move(content)), color(color) {}

public:
    void _draw(const sp<UIContext> &context, const Size &size) override {
        context->saveState();
        context->setFillColor(color);
        content->_draw(context, size);
        context->restoreState();
    }

    auto _size(const sp<UIContext> &context, const ProposedSize &proposed) -> Size override {
        return content->_size(context, proposed);
    }
};

inline auto View::foregroundColor(const Color& color) {
    return sp<ForegroundColor>::of(RetainPtr(this), color);
}