//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include "Shape.hpp"

struct Circle : Shape {
    void path(const sp<UIContext>& context, const Size& size) override {
        float_t halfSize = std::min(size.width, size.height) * 0.5F;
        float_t dx = size.width * 0.5F - halfSize;
        float_t dy = size.height * 0.5F - halfSize;

        context->saveState();
        context->translateBy(dx, dy);
        context->drawCircleFilled(halfSize);
        context->restoreState();
    }
};