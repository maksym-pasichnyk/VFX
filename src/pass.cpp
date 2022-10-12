#include "pass.hpp"
#include "context.hpp"

vfx::RenderPass::RenderPass() {

}

vfx::RenderPass::~RenderPass() {
    context->freeRenderPass(this);
}
