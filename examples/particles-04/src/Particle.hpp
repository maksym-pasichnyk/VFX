#pragma once

#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position = {};
    glm::vec3 velocity = {};
    glm::vec4 color = {};

    float lifetime = 0.0F;
    float time = 0.0F;
};
