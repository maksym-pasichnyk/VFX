#pragma once

#include "Shape.hpp"

struct Rectangle : Shape {
    void path(const sp<Canvas>& canvas, const Size& size) override {
        canvas->drawRectFilled(size);
    }
};