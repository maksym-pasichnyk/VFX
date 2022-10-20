#include "window.hpp"
#include "context.hpp"

vfx::Surface::Surface() {}

vfx::Surface::~Surface() {
    context->instance->destroySurfaceKHR(handle);
}