#include "Surface.hpp"

gfx::SurfaceShared::SurfaceShared(gfx::Instance instance, vk::SurfaceKHR raw)
    : instance(std::move(instance)), raw(raw) {}

gfx::SurfaceShared::~SurfaceShared() {
    instance.shared->raii.raw.destroySurfaceKHR(raw, nullptr, instance.shared->raii.dispatcher);
}

gfx::Surface::Surface() : shared(nullptr) {}
gfx::Surface::Surface(std::shared_ptr<SurfaceShared> shared) : shared(std::move(shared)) {}