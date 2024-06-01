//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include "NotSwiftUI/Views/View.hpp"
#include "NotSwiftUI/Shapes/Shape.hpp"

template<std::derived_from<Shape> T>
struct ShapeView : View {
private:
    rc<T> shape;

public:
    explicit ShapeView(rc<T> shape) : shape(std::move(shape)) {}

public:
    auto getPreferredSize(const ProposedSize &proposed) -> Size override {
        return proposed.orDefault(10.0F, 10.0F);
    }

    void _draw(const rc<Canvas> &canvas, const Size &size) override {
        shape->path(canvas, size);
    }
};

template<std::derived_from<Shape> T>
static auto Shape(rc<T> shape) {
    return rc<struct ShapeView<T>>::init(std::move(shape));
}