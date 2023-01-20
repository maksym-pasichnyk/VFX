#pragma once

#include "Core.hpp"
#include "Iter.hpp"
#include "UIContext.hpp"
#include "Alignment.hpp"

#include "fmt/format.h"

#include <numbers>
#include <numeric>
#include <optional>
#include <algorithm>

struct Border;
struct Overlay;
struct FixedSize;
struct FixedFrame;
struct FlexibleFrame;
struct ForegroundColor;

struct ProposedSize {
    std::optional<float_t> width = {};
    std::optional<float_t> height = {};

    explicit ProposedSize(std::optional<float_t> width, std::optional<float_t> height) : width(width), height(height) {}
    explicit ProposedSize(const UISize& size) : width(size.width), height(size.height) {}

    auto orMax() const -> UISize {
        return UISize(width.value_or(std::numeric_limits<float_t>::max()), height.value_or(std::numeric_limits<float_t>::max()));
    }

    auto orDefault(float_t defaultWidth, float_t defaultHeight) const -> UISize {
        return UISize(width.value_or(defaultWidth), height.value_or(defaultHeight));
    }
};

struct View : Object {
public:
    auto frame(std::optional<float_t> width, std::optional<float_t> height, Alignment alignment = Alignment::center()) -> sp<FixedFrame>;
    auto frame(std::optional<float_t> minWidth, std::optional<float_t> idealWidth, std::optional<float_t> maxWidth, std::optional<float_t> minHeight, std::optional<float_t> idealHeight, std::optional<float_t> maxHeight, Alignment alignment = Alignment::center()) -> sp<FlexibleFrame>;
    auto border(const UIColor& color, float_t width) -> sp<View>;
    auto overlay(sp<View> overlay, Alignment alignment = Alignment::center()) -> sp<Overlay>;
    auto fixedSize(bool horizontal, bool vertical) -> sp<FixedSize>;
    auto foregroundColor(const UIColor& color) -> sp<ForegroundColor>;

public:
    virtual auto body() -> sp<View> {
        throw std::runtime_error("FatalError");
    }
    virtual auto _size(const ProposedSize& proposed) -> UISize {
        return body()->_size(proposed);
    }
    virtual void _draw(const sp<UIContext>& context, const UISize& size) {
        return body()->_draw(context, size);
    }

public:
    auto translation(const UISize& childSize, const UISize& parentSize, const Alignment& alignment) -> UIPoint {
        UIPoint childPoint = alignment.point(childSize);
        UIPoint parentPoint = alignment.point(parentSize);
        return parentPoint - childPoint;
    }
};

struct ViewBuilder {
    static auto arrayOf(auto&&... views) -> std::vector<sp<View>> {
        return {std::forward<decltype(views)>(views)...};
    }
};

struct Text : View {
private:
    std::string text;
    ImFont* font;
    float_t fontSize;

public:
    explicit Text(std::string text, ImFont* font, float_t fontSize)
        : text(std::move(text)), font(font), fontSize(fontSize) {}

public:
    void _draw(const sp<UIContext> &context, const UISize &size) override {
        ImVec2 imSize = font->CalcTextSizeA(fontSize, FLT_MAX, size.width, text.data(), text.data() + text.size(), nullptr);
        UISize uiSize = UISize(imSize.x, imSize.y);
        auto translate = translation(uiSize, size, Alignment::center());

        context->saveState();
        context->translateBy(translate.x, translate.y);
        context->drawText(text, fontSize, font, size.width);
        context->restoreState();
    }

    auto _size(const ProposedSize& proposed) -> UISize override {
        ImVec2 imSize = font->CalcTextSizeA(fontSize, FLT_MAX, proposed.orMax().width, text.data(), text.data() + text.size(), nullptr);
        return proposed.orDefault(imSize.x, imSize.y);
    }
};

struct Shape : View {
    auto body() -> sp<View> override;

    virtual void path(const sp<UIContext>& context, const UISize& size) = 0;
};

struct Circle : Shape {
    void path(const sp<UIContext>& context, const UISize& size) override {
        float_t halfSize = std::min(size.width, size.height) * 0.5F;
        float_t dx = size.width * 0.5F - halfSize;
        float_t dy = size.height * 0.5F - halfSize;

        context->saveState();
        context->translateBy(dx, dy);
        context->drawCircleFilled(halfSize);
        context->restoreState();
    }
};

struct Border : Shape {
private:
    float_t width;

public:
    explicit Border(float_t width) : width(width) {}

    void path(const sp<UIContext>& context, const UISize& size) override {
        context->drawRect(size, width);
    }
};

struct Rectangle : Shape {
    void path(const sp<UIContext>& context, const UISize& size) override {
        context->drawRectFilled(size);
    }
};

template<std::derived_from<Shape> T>
struct ShapeView : View {
private:
    sp<T> shape;

public:
    explicit ShapeView(sp<T> shape) : shape(std::move(shape)) {}

public:
    auto _size(const ProposedSize &proposed) -> UISize override {
        return proposed.orDefault(10.0F, 10.0F);
    }

    void _draw(const sp<UIContext> &context, const UISize &size) override {
        shape->path(context, size);
    }
};

struct FixedFrame : View {
private:
    sp<View> content;
    std::optional<float_t> width;
    std::optional<float_t> height;
    Alignment alignment;

public:
    explicit FixedFrame(sp<View> content, std::optional<float_t> width, std::optional<float_t> height, Alignment alignment)
    : content(std::move(content)), width(width), height(height), alignment(alignment) {}

    auto _size(const ProposedSize& proposed) -> UISize override {
        auto cgSize = proposed.orMax();
        auto childSize = content->_size(ProposedSize(width.value_or(cgSize.width), height.value_or(cgSize.height)));
        return UISize(width.value_or(childSize.width), height.value_or(childSize.height));
    }

    void _draw(const sp<UIContext>& context, const UISize& size) override {
        auto childSize = content->_size(ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        content->_draw(context, childSize);
        context->restoreState();
    }
};

struct FlexibleFrame : View {
private:
    sp<View> content;
    std::optional<float_t> minWidth;
    std::optional<float_t> idealWidth;
    std::optional<float_t> maxWidth;
    std::optional<float_t> minHeight;
    std::optional<float_t> idealHeight;
    std::optional<float_t> maxHeight;
    Alignment alignment;

public:
    explicit FlexibleFrame(sp<View> content, std::optional<float_t> minWidth, std::optional<float_t> idealWidth, std::optional<float_t> maxWidth, std::optional<float_t> minHeight, std::optional<float_t> idealHeight, std::optional<float_t> maxHeight, Alignment alignment)
    : content(std::move(content))
    , minWidth(minWidth)
    , idealWidth(idealWidth)
    , maxWidth(maxWidth)
    , minHeight(minHeight)
    , idealHeight(idealHeight)
    , maxHeight(maxHeight)
    , alignment(alignment) {}

    auto _size(const ProposedSize& p) -> UISize override {
        auto proposed = ProposedSize(p.width ?: idealWidth, p.height ?: idealHeight).orDefault(10.0F, 10.0F);
        if (minWidth.has_value()) {
            proposed.width = std::max(*minWidth, proposed.width);
        }
        if (maxWidth.has_value()) {
            proposed.width = std::min(*maxWidth, proposed.width);
        }
        if (minHeight.has_value()) {
            proposed.height = std::max(*minHeight, proposed.height);
        }
        if (maxHeight.has_value()) {
            proposed.height = std::min(*maxHeight, proposed.height);
        }

        auto size = content->_size(ProposedSize(proposed));
        if (minWidth.has_value()) {
            size.width = std::max(*minWidth, std::min(size.width, proposed.width));
        }
        if (maxWidth.has_value()) {
            size.width = std::min(*maxWidth, std::max(size.width, proposed.width));
        }
        if (minHeight.has_value()) {
            size.height = std::max(*minHeight, std::min(size.height, proposed.height));
        }
        if (maxHeight.has_value()) {
            size.height = std::min(*maxHeight, std::max(size.height, proposed.height));
        }
        return size;
    }

    void _draw(const sp<UIContext>& context, const UISize& size) override {
        auto childSize = content->_size(ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        content->_draw(context, childSize);
        context->restoreState();
    }
};

struct Overlay : View {
private:
    sp<View> content;
    sp<View> overlay;
    Alignment alignment;

public:
    explicit Overlay(sp<View> content, sp<View> overlay, Alignment alignment)
        : content(std::move(content)), overlay(std::move(overlay)), alignment(alignment) {}

    auto _size(const ProposedSize& proposed) -> UISize override {
        return content->_size(proposed);
    }

    void _draw(const sp<UIContext>& context, const UISize& size) override {
        content->_draw(context, size);

        auto childSize = overlay->_size(ProposedSize(size));
        auto translate = translation(childSize, size, alignment);

        context->saveState();
        context->translateBy(translate.x, translate.y);
        overlay->_draw(context, childSize);
        context->restoreState();
    }
};

struct FixedSize : View {
private:
    sp<View> content;
    bool horizontal;
    bool vertical;

public:
    explicit FixedSize(sp<View> content, bool horizontal, bool vertical)
    : content(std::move(content)), horizontal(horizontal), vertical(vertical) {}

    void _draw(const sp<UIContext> &context, const UISize &size) override {
        content->_draw(context, size);
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        auto p = proposed;
        if (horizontal) {
            p.width = std::nullopt;
        }
        if (vertical) {
            p.height = std::nullopt;
        }
        return content->_size(p);
    }
};

struct HStack : View {
private:
    std::vector<sp<View>> children;
    VerticalAlignment alignment;
    float_t spacing;

    std::vector<UISize> sizes = {};

public:
    explicit HStack(std::vector<sp<View>> children, VerticalAlignment alignment = VerticalAlignment::center(), std::optional<float_t> spacing = {})
    : children(std::move(children)), alignment(alignment), spacing(spacing.value_or(0.0F)) {}

    void _draw(const sp<UIContext> &context, const UISize &size) override {
        float_t stackY = alignment.defaultValue(size);
        float_t currentX = 0.0F;

        for (size_t i = 0; i < children.size(); ++i) {
            auto childSize = sizes[i];
            float_t currentY = alignment.defaultValue(childSize);

            context->saveState();
            context->translateBy(currentX, stackY - currentY);
            children[i]->_draw(context, childSize);
            context->restoreState();

            currentX += childSize.width + spacing;
        }
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        if (children.empty()) {
            return UISize(0.0F, 0.0F);
        }

        layout(proposed);

        float_t width = ranges::accumulate(sizes, 0.0F, [](auto $0, auto $1) -> float_t {
            return $0 + $1.width;
        });
        float_t height = ranges::accumulate(sizes, 0.0F, [](auto $0, auto $1) {
            return std::max($0, $1.height);
        });

        width += spacing * static_cast<float_t>(children.size() - 1);

        return UISize(width, height);
    }

    void layout(const ProposedSize &proposed) {
        auto flexibility = cxx::iter(children)
            .map([&](auto& child) {
                auto lower = child->_size(ProposedSize(0, proposed.height));
                auto upper = child->_size(ProposedSize(std::numeric_limits<float_t>::max(), proposed.height));
                return upper.width - lower.width;
            })
            .collect();

        auto remainingIndices = cxx::iter(ranges::views::indices(children.size())).collect();
        ranges::sort(remainingIndices, [&](auto $0, auto $1) {
            return flexibility[$0] < flexibility[$1];
        });

        auto remainingWidth = proposed.width.value() - spacing * static_cast<float_t>(children.size() - 1);

        sizes.resize(children.size(), UISize(0, 0));
        while (!remainingIndices.empty()) {
            auto width = remainingWidth / float_t(remainingIndices.size());

            size_t idx = remainingIndices.front();
            remainingIndices.erase(remainingIndices.begin());

            auto childSize = children[idx]->_size(ProposedSize(width, proposed.height));
            sizes[idx] = childSize;

            remainingWidth -= childSize.width;
            if (remainingWidth < 0) {
                remainingWidth = 0;
            }
        }
    }
};

struct VStack : View {
private:
    std::vector<sp<View>> children;
    HorizontalAlignment alignment;
    float_t spacing;

    std::vector<UISize> sizes = {};

public:
    explicit VStack(std::vector<sp<View>> children, HorizontalAlignment alignment = HorizontalAlignment::center(), std::optional<float_t> spacing = {})
    : children(std::move(children)), alignment(alignment), spacing(spacing.value_or(0.0F)) {}

private:
    void _draw(const sp<UIContext> &context, const UISize &size) override {
        float_t stackX = alignment.defaultValue(size);
        float_t currentY = 0.0F;

        for (size_t i = 0; i < children.size(); ++i) {
            auto childSize = sizes[i];
            float_t currentX = alignment.defaultValue(childSize);

            context->saveState();
            context->translateBy(stackX - currentX, currentY);
            children[i]->_draw(context, childSize);
            context->restoreState();

            currentY += childSize.height + spacing;
        }
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        if (children.empty()) {
            return UISize(0.0F, 0.0F);
        }

        layout(proposed);

        float_t width = ranges::accumulate(sizes, 0.0F, [](auto $0, auto $1) -> float_t {
            return std::max($0, $1.width);
        });
        float_t height = ranges::accumulate(sizes, 0.0F, [](auto $0, auto $1) {
            return $0 + $1.height;
        });

        height += spacing * static_cast<float_t>(children.size() - 1);

        return UISize(width, height);
    }

    void layout(const ProposedSize &proposed) {
        auto flexibility = cxx::iter(children)
            .map([&](auto& child) {
                auto lower = child->_size(ProposedSize(proposed.width, 0));
                auto upper = child->_size(ProposedSize(proposed.width, std::numeric_limits<float_t>::max()));
                return upper.height - lower.height;
            })
            .collect();

        auto remainingIndices = cxx::iter(ranges::views::indices(children.size())).collect();
        ranges::sort(remainingIndices, [&](auto $0, auto $1) {
            return flexibility[$0] < flexibility[$1];
        });

        auto remainingHeight = proposed.height.value() - spacing * static_cast<float_t>(children.size() - 1);

        sizes.resize(children.size(), UISize(0, 0));
        while (!remainingIndices.empty()) {
            auto height = remainingHeight / float_t(remainingIndices.size());

            size_t idx = remainingIndices.front();
            remainingIndices.erase(remainingIndices.begin());

            auto childSize = children[idx]->_size(ProposedSize(proposed.width, height));
            sizes[idx] = childSize;

            remainingHeight -= childSize.height;
            if (remainingHeight < 0) {
                remainingHeight = 0;
            }
        }
    }
};

inline auto View::frame(std::optional<float_t> width, std::optional<float_t> height, Alignment alignment) -> sp<FixedFrame> {
    return sp<FixedFrame>::of(RetainPtr(this), width, height, alignment);
}

inline auto View::frame(std::optional<float_t> minWidth, std::optional<float_t> idealWidth, std::optional<float_t> maxWidth, std::optional<float_t> minHeight, std::optional<float_t> idealHeight, std::optional<float_t> maxHeight, Alignment alignment) -> sp<FlexibleFrame> {
    return sp<FlexibleFrame>::of(RetainPtr(this), minWidth, idealWidth, maxWidth, minHeight, idealHeight, maxHeight, alignment);
}

inline auto View::border(const UIColor& color, float_t width) -> sp<View> {
    return overlay(sp<Border>::of(width)->foregroundColor(color));
}

inline auto View::overlay(sp<View> overlay, Alignment alignment) -> sp<Overlay> {
    return sp<Overlay>::of(RetainPtr(this), overlay, alignment);
}

inline auto View::fixedSize(bool horizontal, bool vertical) -> sp<FixedSize> {
    return sp<FixedSize>::of(RetainPtr(this), horizontal, vertical);
}

struct ForegroundColor : View {
private:
    sp<View> content;
    UIColor color;

public:
    explicit ForegroundColor(sp<View> content, const UIColor& color) : content(content), color(color) {}

public:
    void _draw(const sp<UIContext> &context, const UISize &size) override {
        context->saveState();
        context->setFillColor(color);
        content->_draw(context, size);
        context->restoreState();
    }

    auto _size(const ProposedSize &proposed) -> UISize override {
        return content->_size(proposed);
    }
};

inline auto View::foregroundColor(const UIColor& color) -> sp<ForegroundColor> {
    return sp<ForegroundColor>::of(RetainPtr(this), color);
}

inline auto Shape::body() -> sp<View> {
    return sp<ShapeView<Shape>>::of(RetainPtr(this));
}

