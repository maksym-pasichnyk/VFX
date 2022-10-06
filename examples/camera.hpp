#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct CameraProperties {
    glm::mat4 projection;
    glm::mat4 view;
};

struct Camera {
    static constexpr auto clip = glm::mat4{
        1.0f,  0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 1.0f
    };

    Camera(f32 fov, f32 aspect) : fov(fov), aspect(aspect) {
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);
    }

    void set_aspect(f32 _aspect) {
        aspect = _aspect;
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);
    }

    void set_fov(f32 _fov) {
        fov = _fov;
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);
    }

    auto get_view() const -> const glm::mat4& {
        return view;
    }

    auto get_projection() const -> const glm::mat4& {
        return projection;
    }

private:
    f32 fov = 0.0f;
    f32 aspect = 0.0f;

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
};