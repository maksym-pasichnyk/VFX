#include "Adapter.hpp"
#include "Device.hpp"

gfx::Adapter::Adapter(ManagedShared<Instance> instance, vk::PhysicalDevice gpu)
    : instance(std::move(instance)), gpu(gpu) {}
