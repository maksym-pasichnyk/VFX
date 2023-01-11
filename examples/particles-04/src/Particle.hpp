#pragma once

#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position = {};
    glm::vec3 velocity = {};
    glm::vec4 color = {};

    float_t lifetime = 0.0F;
    float_t time = 0.0F;
};
