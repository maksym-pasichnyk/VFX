#pragma once

#include "vk_mem_alloc.h"
#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Buffer;
    struct Context;
    struct Texture;
    struct Sampler;
    struct Library;
    struct Function;
    struct Drawable;
    struct Swapchain;
    struct CommandQueue;
    struct CommandBuffer;
    struct DescriptorSet;
    struct TextureDescription;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct RenderCommandEncoder;
    struct RenderPipelineStateDescription;

    struct Device final : Referencing {
        friend Buffer;
        friend Context;
        friend Texture;
        friend Sampler;
        friend Library;
        friend Function;
        friend Drawable;
        friend Swapchain;
        friend CommandQueue;
        friend CommandBuffer;
        friend DescriptorSet;
        friend RenderPipelineState;
        friend ComputePipelineState;
        friend RenderPipelineStateDescription;

    private:
        Context* pContext = {};
        VmaAllocator mAllocator = {};

        vk::Device mDevice = {};
        vk::PhysicalDevice mPhysicalDevice = {};
        vk::DispatchLoaderDynamic mDispatchLoaderDynamic = {};

        uint32_t mPresentQueueFamilyIndex = {};
        uint32_t mComputeQueueFamilyIndex = {};
        uint32_t mGraphicsQueueFamilyIndex = {};

    private:
        explicit Device(Context* context, vk::PhysicalDevice gpu);
        ~Device() override;

    public:
        void waitIdle();

        auto newTexture(const TextureDescription& description) -> SharedPtr<Texture>;
        auto newSampler(const vk::SamplerCreateInfo& info) -> SharedPtr<Sampler>;
        auto newBuffer(vk::BufferUsageFlags usage, uint64_t size, VmaAllocationCreateFlags options = 0) -> SharedPtr<Buffer>;
        auto newBuffer(vk::BufferUsageFlags usage, const void* pointer, uint64_t size, VmaAllocationCreateFlags options = 0) -> SharedPtr<Buffer>;
        auto newLibrary(const std::vector<char>& bytes) -> SharedPtr<Library>;
        auto newRenderPipelineState(const RenderPipelineStateDescription& description) -> SharedPtr<RenderPipelineState>;
        auto newComputePipelineState(const SharedPtr<Function>& function) -> SharedPtr<ComputePipelineState>;
        auto newCommandQueue() -> SharedPtr<CommandQueue>;
        auto newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> SharedPtr<DescriptorSet>;
    };
}