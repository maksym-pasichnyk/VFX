#pragma once

#include "Shape.hpp"

struct Rectangle : Shape {
    void path(const rc<Canvas>& canvas, const Size& size) override {
        canvas->drawRectFilled(size);
    }
};