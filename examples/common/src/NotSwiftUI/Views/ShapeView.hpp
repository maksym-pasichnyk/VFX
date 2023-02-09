//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include "NotSwiftUI/Views/View.hpp"
#include "NotSwiftUI/Shapes/Shape.hpp"

template<std::derived_from<Shape> T>
struct ShapeView : View {
private:
    sp<T> shape;

public:
    explicit ShapeView(sp<T> shape) : shape(std::move(shape)) {}

public:
    auto _size(const ProposedSize &proposed) -> Size override {
        return proposed.orDefault(10.0F, 10.0F);
    }

    void _draw(const sp<UIContext> &context, const Size &size) override {
        shape->path(context, size);
    }
};

template<std::derived_from<Shape> T>
static auto Shape(sp<T> shape) {
    return sp<struct ShapeView<T>>::of(std::move(shape));
}