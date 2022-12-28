#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

struct SpvReflectShaderModule;

namespace gfx {
    struct Device;
    struct Function;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct Library final : Referencing<Library> {
        friend Device;
        friend RenderPipelineState;
        friend ComputePipelineState;

    private:
        SharedPtr<Device> mDevice = {};
        vk::ShaderModule vkShaderModule = {};
        SpvReflectShaderModule* spvReflectShaderModule = {};

    private:
        explicit Library(SharedPtr<Device> device, const vk::ShaderModuleCreateInfo& info);
        ~Library() override;

    public:
        auto makeFunction(std::string name) -> SharedPtr<Function>;
    };
}