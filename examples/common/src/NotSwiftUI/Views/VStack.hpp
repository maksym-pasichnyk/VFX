#pragma once

#include "NotSwiftUI/Views/View.hpp"

struct VStack : View {
private:
    std::vector<rc<View>>   children;
    HorizontalAlignment     alignment;
    float_t                 spacing;
    std::vector<Size>       sizes;

public:
    explicit VStack(std::vector<rc<View>> children, HorizontalAlignment alignment = HorizontalAlignment::center(), std::optional<float_t> spacing = {})
        : children(std::move(children)), alignment(alignment), spacing(spacing.value_or(0.0F)) {}

private:
    void _draw(const rc<Canvas> &canvas, const Size &size) override {
        float_t stackX = alignment.defaultValue(size);
        float_t currentY = 0.0F;

        for (size_t i = 0; i < children.size(); ++i) {
            auto childSize = sizes[i];
            float_t currentX = alignment.defaultValue(childSize);

            canvas->saveState();
            canvas->translateBy(stackX - currentX, currentY);
            children[i]->_draw(canvas, childSize);
            canvas->restoreState();

            currentY += childSize.height + spacing;
        }
    }

    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        if (children.empty()) {
            return Size::zero();
        }

        layout(proposed);

        float_t width = ranges::accumulate(sizes, 0.0F, [](auto lhs, auto rhs) -> float_t {
            return std::max(lhs, rhs.width);
        });
        float_t height = ranges::accumulate(sizes, 0.0F, [](auto lhs, auto rhs) {
            return lhs + rhs.height;
        });

        height += spacing * static_cast<float_t>(children.size() - 1);

        return Size{width, height};
    }

    void layout(const ProposedSize &proposed) {
        auto flexibility = cxx::iter(children)
            .map([&](auto& child) {
                auto lower = child->getPreferredSize(ProposedSize(proposed.width, 0));
                auto upper = child->getPreferredSize(ProposedSize(proposed.width, std::numeric_limits<float_t>::max()));
                return upper.height - lower.height;
            })
            .collect();

        auto remainingIndices = cxx::iter(ranges::views::indices(children.size())).collect();
        ranges::sort(remainingIndices, [&](auto lhs, auto rhs) {
            return flexibility[lhs] < flexibility[rhs];
        });

        auto remainingHeight = proposed.height.value() - spacing * static_cast<float_t>(children.size() - 1);

        sizes.resize(children.size(), Size::zero());
        while (!remainingIndices.empty()) {
            auto height = remainingHeight / float_t(remainingIndices.size());

            size_t idx = remainingIndices.front();
            remainingIndices.erase(remainingIndices.begin());

            auto childSize = children[idx]->getPreferredSize(ProposedSize(proposed.width, height));
            sizes[idx] = childSize;

            remainingHeight -= childSize.height;
            if (remainingHeight < 0) {
                remainingHeight = 0;
            }
        }
    }
};

static auto VStack(HorizontalAlignment alignment, std::optional<float_t> spacing, std::vector<rc<View>> children) {
    return rc<struct VStack>::init(std::move(children), alignment, spacing);
}