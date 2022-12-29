#pragma once

#include "vk_mem_alloc.h"
#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Buffer;
    struct Texture;
    struct Sampler;
    struct Library;
    struct Function;
    struct Drawable;
    struct Swapchain;
    struct Application;
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
        friend Texture;
        friend Sampler;
        friend Library;
        friend Function;
        friend Drawable;
        friend Swapchain;
        friend Application;
        friend CommandQueue;
        friend CommandBuffer;
        friend DescriptorSet;
        friend TextureDescription;
        friend RenderPipelineState;
        friend ComputePipelineState;
        friend RenderCommandEncoder;
        friend RenderPipelineStateDescription;

    private:
        Application* pApplication;
        VmaAllocator vmaAllocator = {};

        vk::Device vkDevice = {};
        vk::PhysicalDevice vkPhysicalDevice = {};
        vk::DispatchLoaderDynamic vkDispatchLoaderDynamic = {};

        uint32_t vkPresentQueueFamilyIndex = {};
        uint32_t vkComputeQueueFamilyIndex = {};
        uint32_t vkGraphicsQueueFamilyIndex = {};

    private:
        explicit Device(Application* pApplication, vk::PhysicalDevice gpu);
        ~Device() override;

    public:
        void waitIdle();

        auto newTexture(const TextureDescription& description) -> SharedPtr<Texture>;
        auto newSampler(const vk::SamplerCreateInfo& info) -> SharedPtr<Sampler>;
        auto newBuffer(vk::BufferUsageFlags usage, uint64_t size, VmaAllocationCreateFlags options = 0) -> SharedPtr<Buffer>;
        auto newBuffer(vk::BufferUsageFlags usage, uint64_t size, const void* data, VmaAllocationCreateFlags options = 0) -> SharedPtr<Buffer>;
        auto newLibrary(const std::vector<char>& bytes) -> SharedPtr<Library>;
        auto newRenderPipelineState(const RenderPipelineStateDescription& description) -> SharedPtr<RenderPipelineState>;
        auto newComputePipelineState(const SharedPtr<Function>& function) -> SharedPtr<ComputePipelineState>;
        auto newCommandQueue() -> SharedPtr<CommandQueue>;
        auto newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> SharedPtr<DescriptorSet>;
    };
}