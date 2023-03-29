#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct Text : View {
private:
    std::string text;
    float_t fontSize;

public:
    explicit Text(std::string text, float_t fontSize)
        : text(std::move(text)), fontSize(fontSize) {}

public:
    void _draw(const sp<UIContext> &context, const Size &size) override {
        auto font = context->drawList()->_Data->Font;

        auto imSize = font->CalcTextSizeA(fontSize, FLT_MAX, size.width, text.data(), text.data() + text.size(), nullptr);
        auto uiSize = Size{imSize.x, imSize.y};
        auto translate = translation(uiSize, size, Alignment::center());

        context->saveState();
        context->translateBy(translate.x, translate.y);
        context->drawText(text, fontSize, font, size.width);
        context->restoreState();
    }

    auto _size(const sp<UIContext> &context, const ProposedSize& proposed) -> Size override {
        auto font = context->drawList()->_Data->Font;
        auto imSize = font->CalcTextSizeA(fontSize, FLT_MAX, proposed.orMax().width, text.data(), text.data() + text.size(), nullptr);
        return proposed.orDefault(imSize.x, imSize.y);
    }
};

static auto Text(std::string text, float_t fontSize) {
    return sp<struct Text>::of(std::move(text), fontSize);
}