#pragma once

#include "UIContext.hpp"
#include "Alignment.hpp"

#include <optional>

using ProposedViewSize = UISize;

struct Border;
struct FixedFrame;

struct View : gfx::Referencing {
    virtual auto size(const ProposedViewSize& proposed) -> UISize {
        return {proposed.width, proposed.height};
    }
    virtual void draw(const gfx::SharedPtr<UIContext>& context, const UIPoint& origin, const ProposedViewSize& proposed) {}

    auto frame(float_t width, float_t height, Alignment alignment = Alignment::center()) -> gfx::SharedPtr<FixedFrame>;
    auto border(UIColor color, float_t width) -> gfx::SharedPtr<Border>;
};

struct Rectangle : View {
    void draw(const gfx::SharedPtr<UIContext>& context, const UIPoint& origin, const ProposedViewSize& proposed) override {
        float_t x0 = origin.x;
        float_t y0 = origin.y;
        float_t x1 = origin.x + proposed.width;
        float_t y1 = origin.y + proposed.height;
        context->draw_list.AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), ImColor(1.0F, 1.0F, 1.0F, 1.0F));
    }
};

struct Border : View {
    UIColor mColor = {};
    float_t mWidth = {};
    gfx::SharedPtr<View> mContent = {};

    auto size(const ProposedViewSize &proposed) -> UISize override {
        return mContent->size(proposed);
    }

    void draw(const gfx::SharedPtr<UIContext>& context, const UIPoint& origin, const ProposedViewSize& proposed) override {
        mContent->draw(context, origin, proposed);

        float_t x0 = origin.x;
        float_t y0 = origin.y;
        float_t x1 = origin.x + proposed.width;
        float_t y1 = origin.y + proposed.height;
        context->draw_list.AddRect(ImVec2(x0, y0), ImVec2(x1, y1), ImColor(mColor.r, mColor.g, mColor.b, mColor.a), 0.0F, 0, mWidth);
    }
};

struct FixedFrame : View {
    std::optional<float_t> mWidth = {};
    std::optional<float_t> mHeight = {};
    gfx::SharedPtr<View> mContent = {};
    Alignment mAlignment = Alignment::center();

    auto size(const ProposedViewSize& proposed) -> UISize override {
        float_t width = mWidth.value_or(proposed.width);
        float_t height = mHeight.value_or(proposed.height);

        auto childSize = mContent->size({width, height});
        return UISize{
            mWidth.value_or(childSize.width),
            mHeight.value_or(childSize.height)
        };
    }

    void draw(const gfx::SharedPtr<UIContext>& context, const UIPoint& origin, const ProposedViewSize& proposed) override {
        auto childSize = mContent->size(proposed);

        UIPoint selfPoint = mAlignment.point(proposed);
        UIPoint childPoint = mAlignment.point(childSize);

        UIPoint point = origin;
        point.x += selfPoint.x - childPoint.x;
        point.y += selfPoint.y - childPoint.y;
        mContent->draw(context, point, childSize);
    }
};

auto View::frame(float_t width, float_t height, Alignment alignment) -> gfx::SharedPtr<FixedFrame> {
    auto view = gfx::TransferPtr(new FixedFrame());
    view->mContent = gfx::RetainPtr(this);
    view->mWidth = width;
    view->mHeight = height;
    view->mAlignment = alignment;
    return view;
}

auto View::border(UIColor color, float_t width) -> gfx::SharedPtr<Border> {
    auto view = gfx::TransferPtr(new Border());
    view->mContent = gfx::RetainPtr(this);
    view->mWidth = width;
    view->mColor = color;
    return view;
}