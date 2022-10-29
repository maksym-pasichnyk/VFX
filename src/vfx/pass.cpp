#include "pass.hpp"
#include "device.hpp"

vfx::RenderPass::RenderPass() {}

vfx::RenderPass::~RenderPass() {
    device->freeRenderPass(this);
}
