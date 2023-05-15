//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include "Shape.hpp"

struct Circle : Shape {
    void path(const ManagedShared<Canvas>& canvas, const Size& size) override {
        float_t halfSize = std::min(size.width, size.height) * 0.5F;
        float_t dx = size.width * 0.5F - halfSize;
        float_t dy = size.height * 0.5F - halfSize;

        canvas->saveState();
        canvas->translateBy(dx, dy);
        canvas->drawCircleFilled(halfSize);
        canvas->restoreState();
    }
};