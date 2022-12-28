#pragma once

#include "Object.hpp"

#include <string>

namespace gfx {
    struct Device;
    struct Library;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct Function final : Referencing<Function> {
        friend Device;
        friend Library;
        friend RenderPipelineState;
        friend ComputePipelineState;

    private:
        SharedPtr<Library> mLibrary;
        std::string mFunctionName;

    private:
        explicit Function(SharedPtr<Library> library, std::string name);
    };
}