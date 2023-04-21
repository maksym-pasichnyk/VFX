#pragma once

#include "NotSwiftUI/Views/View.hpp"

#include "Iter.hpp"

struct HStack : View {
private:
    std::vector<sp<View>> children;
    VerticalAlignment alignment;
    float_t spacing;

    std::vector<Size> sizes = {};

public:
    explicit HStack(std::vector<sp<View>> children, VerticalAlignment alignment = VerticalAlignment::center(), std::optional<float_t> spacing = {})
        : children(std::move(children)), alignment(alignment), spacing(spacing.value_or(0.0F)) {}

    void _draw(const sp<Canvas> &canvas, const Size &size) override {
        float_t stackY = alignment.defaultValue(size);
        float_t currentX = 0.0F;

        for (size_t i = 0; i < children.size(); ++i) {
            auto childSize = sizes[i];
            float_t currentY = alignment.defaultValue(childSize);

            canvas->saveState();
            canvas->translateBy(currentX, stackY - currentY);
            children[i]->_draw(canvas, childSize);
            canvas->restoreState();

            currentX += childSize.width + spacing;
        }
    }

    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        if (children.empty()) {
            return Size::zero();
        }

        layout(proposed);

        float_t width = ranges::accumulate(sizes, 0.0F, [](auto lhs, auto rhs) -> float_t {
            return lhs + rhs.width;
        });
        float_t height = ranges::accumulate(sizes, 0.0F, [](auto lhs, auto rhs) {
            return std::max(lhs, rhs.height);
        });

        width += spacing * static_cast<float_t>(children.size() - 1);

        return Size{width, height};
    }

    void layout(const ProposedSize &proposed) {
        auto flexibility = cxx::iter(children)
            .map([&](auto& child) {
                auto lower = child->getPreferredSize(ProposedSize(0, proposed.height));
                auto upper = child->getPreferredSize(ProposedSize(std::numeric_limits<float_t>::max(), proposed.height));
                return upper.width - lower.width;
            })
            .collect();

        auto remainingIndices = cxx::iter(ranges::views::indices(children.size())).collect();
        ranges::sort(remainingIndices, [&](auto lhs, auto rhs) {
            return flexibility[lhs] < flexibility[rhs];
        });

        auto remainingWidth = proposed.width.value() - spacing * static_cast<float_t>(children.size() - 1);

        sizes.resize(children.size(), Size::zero());
        while (!remainingIndices.empty()) {
            auto width = remainingWidth / float_t(remainingIndices.size());

            size_t idx = remainingIndices.front();
            remainingIndices.erase(remainingIndices.begin());

            auto childSize = children[idx]->getPreferredSize(ProposedSize(width, proposed.height));
            sizes[idx] = childSize;

            remainingWidth -= childSize.width;
            if (remainingWidth < 0) {
                remainingWidth = 0;
            }
        }
    }
};

static auto HStack(VerticalAlignment alignment, std::optional<float_t> spacing, std::vector<sp<View>> children) {
    return sp<struct HStack>::of(std::move(children), alignment, spacing);
}