#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct BakedVertex {
    glm::vec3 position = {};
    glm::vec3 normal   = {};
    glm::vec4 color    = {};
    glm::vec2 texcoord = {};
};
