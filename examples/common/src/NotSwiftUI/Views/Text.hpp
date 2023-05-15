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
    void _draw(const sp<Canvas> &canvas, const Size &size) override {
        auto imSize = ImGui::GetDefaultFont()->CalcTextSizeA(fontSize, FLT_MAX, size.width, text.data(), text.data() + text.size(), nullptr);
        auto uiSize = Size{imSize.x, imSize.y};
        auto translate = translation(uiSize, size, Alignment::center());

        canvas->saveState();
        canvas->translateBy(translate.x, translate.y);
        canvas->drawText(text, fontSize, ImGui::GetDefaultFont(), size.width);
        canvas->restoreState();
    }

    auto getPreferredSize(const ProposedSize& proposed) -> Size override {
        auto imSize = ImGui::GetDefaultFont()->CalcTextSizeA(fontSize, FLT_MAX, proposed.orMax().width, text.data(), text.data() + text.size(), nullptr);
        return proposed.orDefault(imSize.x, imSize.y);
    }
};

static auto Text(std::string text, float_t fontSize) {
    return MakeShared<struct Text>(std::move(text), fontSize);
}