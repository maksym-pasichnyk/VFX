#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Camera {
public:
    static auto getInfinityProjectionMatrix(float fov, float aspect, float zNear) -> glm::mat4 {
        f32 range = glm::tan(glm::radians(fov) / 2.0f);
        f32 a = +1.0f / (range * aspect);
        f32 b = -1.0f / (range);
        f32 c = zNear;

        return {
            glm::vec4(a, 0, 0, 0),
            glm::vec4(0, b, 0, 0),
            glm::vec4(0, 0, 0, 1),
            glm::vec4(0, 0, c, 0)
        };
    }

public:
    void setProjectionMatrix(const glm::mat4& m) {
        projectionMatrix = m;
    }

    [[nodiscard]]
    auto getProjectionMatrix() const -> const glm::mat4& {
        return projectionMatrix;
    }

    void setWorldToCameraMatrix(const glm::mat4& m) {
        worldToCameraMatrix = m;
    }

    [[nodiscard]]
    auto getWorldToCameraMatrix() const -> const glm::mat4& {
        return worldToCameraMatrix;
    }

private:
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 worldToCameraMatrix = glm::mat4(1.0f);
};