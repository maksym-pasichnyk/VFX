#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct Overlay : View {
private:
    rc<View> content;
    rc<View> overlay;
    Alignment alignment;

public:
    explicit Overlay(rc<View> content, rc<View> overlay, Alignment alignment)
        : content(std::move(content)), overlay(std::move(overlay)), alignment(alignment) {}

    auto getPreferredSize(const ProposedSize& proposed) -> Size override {
        return content->getPreferredSize(proposed);
    }

    void _draw(const rc<Canvas>& canvas, const Size& size) override {
        content->_draw(canvas, size);

        auto childSize = overlay->getPreferredSize(ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        canvas->saveState();
        canvas->translateBy(translate.x, translate.y);
        overlay->_draw(canvas, childSize);
        canvas->restoreState();
    }
};

inline auto View::overlay(rc<View> overlay, Alignment alignment) {
    return rc<Overlay>::init(shared_from_this(), std::move(overlay), alignment);
}