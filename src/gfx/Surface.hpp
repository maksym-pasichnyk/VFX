#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Surface : public ManagedObject {
        rc<Instance>   instance;
        vk::SurfaceKHR handle;

        explicit Surface(rc<Instance> instance, vk::SurfaceKHR handle);
        ~Surface() override;
    };
}