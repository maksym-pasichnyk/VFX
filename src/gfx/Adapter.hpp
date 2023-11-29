#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Adapter : public ManagedObject {
        rc<Instance>        instance;
        vk::PhysicalDevice  handle;

        explicit Adapter(rc<Instance> instance, vk::PhysicalDevice handle);

        auto getSurfaceCapabilities(this Adapter& self, rc<Surface> const& surface) -> vk::SurfaceCapabilitiesKHR;
        auto createDevice(this Adapter& self, vk::DeviceCreateInfo const& create_info) -> rc<Device>;
    };
}