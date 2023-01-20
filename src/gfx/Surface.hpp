#pragma once

#include "Instance.hpp"

namespace gfx {
    struct SurfaceShared {
        Instance instance;
        vk::SurfaceKHR raw;

        explicit SurfaceShared(Instance instance, vk::SurfaceKHR raw);
        ~SurfaceShared();
    };

    struct Surface {
        std::shared_ptr<SurfaceShared> shared;

        explicit Surface();
        explicit Surface(std::shared_ptr<SurfaceShared> shared);
    };
}