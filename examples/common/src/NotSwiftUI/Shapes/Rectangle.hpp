#pragma once

#include "Shape.hpp"

struct Rectangle : Shape {
    void path(const ManagedShared<Canvas>& canvas, const Size& size) override {
        canvas->drawRectFilled(size);
    }
};