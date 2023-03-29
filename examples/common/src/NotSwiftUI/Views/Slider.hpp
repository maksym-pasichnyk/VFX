#pragma once

#include "NotSwiftUI/Views/View.hpp"
#include "NotSwiftUI/Modifiers/FixedSize.hpp"

struct Slider : View {
private:
    float_t min;
    float_t max;
    float_t current;

public:
    Slider(float_t current, float_t min, float_t max)
        : min(min), max(max), current(current) {}

    void _draw(const sp<UIContext>& ctx, const Size &size) override {
        float sliderHeight = 4.0F;
        float sliderWidth = size.width;
        float knobRadius = 8.0F;
        float sliderHalfHeight = sliderHeight * 0.5F;

        auto translate = translation(Size{sliderWidth, sliderHeight}, size, Alignment::center());

        ctx->saveState();
        ctx->translateBy(translate.x, translate.y);
        ctx->translateBy(0.0F, -sliderHalfHeight);
        ctx->drawRectFilled(Size{sliderWidth, sliderHeight}, 0.0F);
        ctx->translateBy(0.0F, +sliderHalfHeight);
        ctx->setFillColor(Color{1.0F, 0.0F, 0.0F, 1.0F});
        ctx->translateBy(-knobRadius, -knobRadius);
        float knobX = (current - min) / (max - min) * sliderWidth;
        ctx->translateBy(+knobX, 0.0F);
        ctx->drawCircleFilled(knobRadius);
        ctx->translateBy(-knobX, 0.0F);
        ctx->translateBy(+knobRadius, +knobRadius);
        ctx->restoreState();
    }

    auto _size(const sp<UIContext> &context, const ProposedSize &proposed) -> Size override {
        return proposed.orDefault(200.0F, 20.0F);
    }
};

static auto Slider(float_t current, float_t min, float_t max) {
    return sp<struct Slider>::of(current, min, max)->fixedSize(false, true);
}