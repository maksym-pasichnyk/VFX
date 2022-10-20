#pragma once

#include "Core.hpp"
#include "Math.hpp"

#include <vector>

struct DrawVertex {
    glm::vec3 position = {};
    u32       color    = {};
};

struct DrawList {
    std::vector<u32> indices = {};
    std::vector<DrawVertex> vertices = {};

    void clear();
    void addQuad(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, u32 color);
    void drawBox(const glm::vec3& from, const glm::vec3& to, u32 color);
};