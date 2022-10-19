#pragma once

#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Camera {
public:
    static auto getInfinityProjectionMatrix(float fov, float aspect, float zNear) {
        f32 range = glm::tan(glm::radians(fov) / 2.0f);

        glm::mat4 out(0.0f);
        out[0][0] =  1.0f / (range * aspect);
        out[1][1] = -1.0f / (range);
        out[2][2] =  0.0f;
        out[2][3] =  1.0f;
        out[3][2] =  zNear;
        return out;
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