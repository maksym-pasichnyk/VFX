#include "DrawList.hpp"

void DrawList::clear() {
    indices.clear();
    vertices.clear();
}

void DrawList::addQuad(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, u32 color) {
    indices.reserve(6);
    for (i32 i : {0, 1, 2, 0, 2, 3}) {
        indices.emplace_back(i + vertices.size());
    }

    vertices.reserve(4);
    vertices.emplace_back(DrawVertex{.position = p0, .color = color});
    vertices.emplace_back(DrawVertex{.position = p1, .color = color});
    vertices.emplace_back(DrawVertex{.position = p2, .color = color});
    vertices.emplace_back(DrawVertex{.position = p3, .color = color});
}

void DrawList::drawBox(const glm::vec3& from, const glm::vec3& to, u32 color) {
    auto const min = glm::min(from, to);
    auto const max = glm::max(from, to);

    // front
    addQuad(
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, max.y, max.z),
        glm::vec3(min.x, max.y, max.z),
        color
    );

    // back
    addQuad(
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(min.x, max.y, min.z),
        color
    );

    // right
    addQuad(
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, max.y, max.z),
        glm::vec3(max.x, max.y, min.z),
        color
    );

    // left
    addQuad(
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(min.x, max.y, min.z),
        color
    );

    // up
    addQuad(
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z),
        glm::vec3(max.x, max.y, min.z),
        color
    );

    // down
    addQuad(
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(max.x, min.y, min.z),
        color
    );
}
