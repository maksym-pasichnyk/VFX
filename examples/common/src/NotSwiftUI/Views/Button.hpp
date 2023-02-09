#pragma once

#include "NotSwiftUI/Views/Text.hpp"

struct Button : View {
private:
    sp<struct Text> text;

public:
    explicit Button(sp<struct Text> text) : text(std::move(text)) {}

    void _draw(const sp<UIContext> &context, const Size &size) override {
        auto textSize = text->_size(ProposedSize(size));
        auto translate = translation(textSize, size, Alignment::center());

        context->drawRectFilled(size, 5.0F);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        context->setFillColor(Color{0.0F, 0.0F, 0.0F, 1.0F});
        text->_draw(context, textSize);
        context->restoreState();
    }

    auto _size(const ProposedSize &proposed) -> Size override {
        auto textSize = text->_size(proposed);
        return proposed.orDefault(textSize.width, textSize.height);
    }
};

static auto Button(sp<struct Text> text) {
    return sp<struct Button>::of(std::move(text));
}