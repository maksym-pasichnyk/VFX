#include "Surface.hpp"

gfx::Surface::Surface(ManagedShared<Instance> instance, vk::SurfaceKHR raw)
    : instance(std::move(instance)), raw(raw) {}

gfx::Surface::~Surface() {
    instance->raii.raw.destroySurfaceKHR(raw, nullptr, instance->raii.dispatcher);
}