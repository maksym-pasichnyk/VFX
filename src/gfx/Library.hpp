#pragma once

#include "Object.hpp"

#include <map>
#include <spirv_reflect.h>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Library;
    struct Function;
    struct RenderPipelineState;
    struct ComputePipelineState;

    struct Library final : Referencing<Library> {
        friend Device;
        friend Function;
        friend RenderPipelineState;
        friend ComputePipelineState;

    private:
        SharedPtr<Device> mDevice = {};
        vk::ShaderModule vkShaderModule = {};
        SpvReflectShaderModule mSpvReflectShaderModule = {};

    private:
        explicit Library(SharedPtr<Device> device, const vk::ShaderModuleCreateInfo& info);
        ~Library() override;

    public:
        auto newFunction(std::string name) -> SharedPtr<Function>;
    };
}