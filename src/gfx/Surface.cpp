#include "Surface.hpp"

gfx::Surface::Surface(rc<Instance> instance, vk::SurfaceKHR handle)
    : instance(std::move(instance)), handle(handle) {}

gfx::Surface::~Surface() {
    instance->handle.destroySurfaceKHR(handle, nullptr, instance->dispatcher);
}