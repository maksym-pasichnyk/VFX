#include "material.hpp"
#include "context.hpp"

vfx::PipelineState::PipelineState() {}

vfx::PipelineState::~PipelineState() {
    context->freePipelineState(this);
}
