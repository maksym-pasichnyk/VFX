#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Device;

    struct Adapter : ManagedObject<Adapter> {
        friend Instance;

    private:
        ManagedShared<Instance> instance    = {};
        vk::PhysicalDevice      gpu         = {};

    private:
        explicit Adapter(ManagedShared<Instance> instance, vk::PhysicalDevice gpu);
    };
}