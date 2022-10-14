#include "material.hpp"
#include "context.hpp"

vfx::Function::Function() {}

vfx::Function::~Function() {
    context->freeFunction(this);
}

vfx::PipelineState::PipelineState() {}

vfx::PipelineState::~PipelineState() {
    context->freePipelineState(this);
}
