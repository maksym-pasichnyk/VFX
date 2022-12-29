#include "Function.hpp"
#include "Library.hpp"

gfx::Function::Function(SharedPtr<Library> library, std::string name) : mLibrary(std::move(library)), mFunctionName(std::move(name)) {
    for (auto& sep : std::span(mLibrary->mSpvReflectShaderModule.entry_points, mLibrary->mSpvReflectShaderModule.entry_point_count)) {
        if (mFunctionName == sep.name) {
            mEntryPoint = &sep;
            return;
        }
    }
    throw std::runtime_error("Entry point not found");
}