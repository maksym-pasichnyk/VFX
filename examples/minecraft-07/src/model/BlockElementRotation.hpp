#pragma once

#include "block/Axis.hpp"

struct BlockElementRotation {
    glm::vec3 origin;
    Axis axis;
    float_t angle;
    bool rescale;

    explicit BlockElementRotation(const glm::vec3& origin, Axis axis, float_t angle, bool rescale)
        : origin(origin), axis(axis), angle(angle), rescale(rescale) {}
};
