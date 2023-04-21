#pragma once

#include "NotSwiftUI/Core/Size.hpp"
#include "Canvas.hpp"

struct Shape : Object {
    virtual void path(const sp<Canvas>& canvas, const Size& size) = 0;
};