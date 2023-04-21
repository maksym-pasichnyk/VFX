#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct FixedSize : View {
private:
    sp<View> content;
    bool horizontal;
    bool vertical;

public:
    explicit FixedSize(sp<View> content, bool horizontal, bool vertical)
        : content(std::move(content)), horizontal(horizontal), vertical(vertical) {}

    void _draw(const sp<Canvas> &canvas, const Size &size) override {
        content->_draw(canvas, size);
    }

    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        auto p = proposed;
        if (horizontal) {
            p.width = std::nullopt;
        }
        if (vertical) {
            p.height = std::nullopt;
        }
        return content->getPreferredSize(p);
    }
};

inline auto View::fixedSize(bool horizontal, bool vertical) {
    return sp<FixedSize>::of(RetainPtr(this), horizontal, vertical);
}
