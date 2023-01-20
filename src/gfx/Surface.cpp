#include "Surface.hpp"

gfx::SurfaceShared::SurfaceShared(gfx::Instance instance, vk::SurfaceKHR raw)
    : instance(std::move(instance)), raw(raw) {}

gfx::SurfaceShared::~SurfaceShared() {
    instance.handle().destroySurfaceKHR(raw, nullptr, instance.dispatcher());
}

gfx::Surface::Surface() : shared(nullptr) {}
gfx::Surface::Surface(std::shared_ptr<SurfaceShared> shared) : shared(std::move(shared)) {}