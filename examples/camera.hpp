#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Camera {
private:
    static constexpr auto clip = glm::mat4{
        1.0f,  0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 1.0f
    };

public:
    [[nodiscard]]
    static auto getInfinityProjectionMatrix(f32 fov, f32 aspect, f32 zNear) -> glm::mat4 {
        return clip * glm::infinitePerspectiveLH(glm::radians(fov), aspect, zNear);
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