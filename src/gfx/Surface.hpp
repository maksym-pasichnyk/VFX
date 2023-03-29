#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Surface : ManagedObject<Surface> {
        ManagedShared<Instance> instance;
        vk::SurfaceKHR          raw;

        explicit Surface(ManagedShared<Instance> instance, vk::SurfaceKHR raw);
        ~Surface() override;
    };
}