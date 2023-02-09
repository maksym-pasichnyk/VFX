#pragma once

#include "Shape.hpp"

struct Rectangle : Shape {
    void path(const sp<UIContext>& context, const Size& size) override {
        context->drawRectFilled(size);
    }
};