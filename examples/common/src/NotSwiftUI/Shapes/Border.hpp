#pragma once

#include "Shape.hpp"
#include "NotSwiftUI/Views/Overlay.hpp"
#include "NotSwiftUI/Views/ShapeView.hpp"
#include "NotSwiftUI/Modifiers/ForegroundColor.hpp"

struct Border : Shape {
private:
    float_t width;

public:
    explicit Border(float_t width) : width(width) {}

    void path(const rc<Canvas>& canvas, const Size& size) override {
        canvas->drawRect(size, width);
    }
};

inline auto View::border(const Color& color, float_t width) {
    return overlay(rc<ShapeView<Border>>::init(rc<Border>::init(width))->foregroundColor(color));
}