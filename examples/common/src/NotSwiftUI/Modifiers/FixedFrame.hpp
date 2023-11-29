#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct FixedFrame : View {
private:
    rc<View> content;
    std::optional<float_t> width;
    std::optional<float_t> height;
    Alignment alignment;

public:
    explicit FixedFrame(rc<View> content, std::optional<float_t> width, std::optional<float_t> height, Alignment alignment)
        : content(std::move(content)), width(width), height(height), alignment(alignment) {}

    auto getPreferredSize(const ProposedSize& proposed) -> Size override {
        auto cgSize = proposed.orMax();
        auto childSize = content->getPreferredSize(ProposedSize(width.value_or(cgSize.width), height.value_or(cgSize.height)));
        return Size{width.value_or(childSize.width), height.value_or(childSize.height)};
    }

    void _draw(const rc<Canvas>& canvas, const Size& size) override {
        auto childSize = content->getPreferredSize(ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        canvas->saveState();
        canvas->translateBy(translate.x, translate.y);
        content->_draw(canvas, childSize);
        canvas->restoreState();
    }
};

inline auto View::frame(std::optional<float_t> width, std::optional<float_t> height, Alignment alignment) {
    return MakeShared<FixedFrame>(shared_from_this(), width, height, alignment);
}