#pragma once

#include "NotSwiftUI/Core/Size.hpp"
#include "Canvas.hpp"

struct Shape : public ManagedObject {
    virtual void path(const rc<Canvas>& canvas, const Size& size) = 0;
};