#pragma once

#include "Object.hpp"
#include "spirv_reflect.h"

#include <string>

namespace gfx {
    struct Device;
    struct Library;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct Function final : Referencing {
        friend Device;
        friend Library;
        friend RenderPipelineState;
        friend ComputePipelineState;

    private:
        std::string mFunctionName;
        SharedPtr<Library> mLibrary;
        SpvReflectEntryPoint* mEntryPoint = {};

    private:
        explicit Function(SharedPtr<Library> library, std::string name);
    };
}