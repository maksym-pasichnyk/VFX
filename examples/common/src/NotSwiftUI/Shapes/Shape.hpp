#pragma once

#include "NotSwiftUI/Core/Size.hpp"
#include "UIContext.hpp"

struct Shape : Object {
    virtual void path(const sp<UIContext>& context, const Size& size) = 0;
};