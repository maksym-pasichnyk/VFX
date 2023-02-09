#pragma once

#include "BakedQuad.hpp"
#include "block/Direction.hpp"

struct BakedModel : Object {
    struct Consumer : std::function<void(const BakedQuad&)> {
        using function::function;
    };

    virtual ~BakedModel() = default;

    virtual void drawQuads(std::optional<Direction> facing, const Consumer& consumer) = 0;
};