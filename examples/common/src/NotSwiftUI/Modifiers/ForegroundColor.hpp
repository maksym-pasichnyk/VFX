#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct ForegroundColor : View {
private:
    rc<View> content;
    Color color;

public:
    explicit ForegroundColor(rc<View> content, const Color& color)
        : content(std::move(content)), color(color) {}

public:
    void _draw(const rc<Canvas> &canvas, const Size &size) override {
        canvas->saveState();
        canvas->setFillColor(color);
        content->_draw(canvas, size);
        canvas->restoreState();
    }

    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        return content->getPreferredSize(proposed);
    }
};

inline auto View::foregroundColor(const Color& color) {
    return MakeShared<ForegroundColor>(shared_from_this(), color);
}