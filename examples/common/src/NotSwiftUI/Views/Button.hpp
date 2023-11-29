#pragma once

#include "NotSwiftUI/Views/Text.hpp"

struct Button : View {
private:
    rc<struct View> text;

public:
    explicit Button(rc<struct View> text) : text(std::move(text)) {}

    void _draw(const rc<Canvas> &canvas, const Size &size) override {
        auto textSize = text->getPreferredSize(ProposedSize(size));
        auto translate = translation(textSize, size, Alignment::center());

        canvas->drawRectFilled(size, 5.0F);

        canvas->saveState();
        canvas->translateBy(translate.x, translate.y);
        canvas->setFillColor(Color{0.0F, 0.0F, 0.0F, 1.0F});
        text->_draw(canvas, textSize);
        canvas->restoreState();
    }

    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        auto textSize = text->getPreferredSize(proposed);
        return proposed.orDefault(textSize.width, textSize.height);
    }
};

static auto Button(rc<struct View> text) {
    return MakeShared<struct Button>(std::move(text));
}