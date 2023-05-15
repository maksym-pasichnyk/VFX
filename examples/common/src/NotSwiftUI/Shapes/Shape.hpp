#pragma once

#include "NotSwiftUI/Core/Size.hpp"
#include "Canvas.hpp"

struct Shape : ManagedObject<Shape> {
    virtual void path(const ManagedShared<Canvas>& canvas, const Size& size) = 0;
};