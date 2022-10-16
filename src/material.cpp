#include "material.hpp"
#include "context.hpp"

vfx::Library::Library() {}

vfx::Library::~Library() {
    context->freeLibrary(this);
}

auto vfx::Library::makeFunction(std::string name) -> Arc<Function> {
    auto out = Arc<Function>::alloc();
    out->library = shared_from_this();
    out->name = std::move(name);
    return out;
}

vfx::PipelineState::PipelineState() {}

vfx::PipelineState::~PipelineState() {
    context->freePipelineState(this);
}
