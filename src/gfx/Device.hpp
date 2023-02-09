#pragma once

#include "Instance.hpp"

namespace gfx {
    struct Buffer;
    struct Texture;
    struct Surface;
    struct Sampler;
    struct Library;
    struct Function;
    struct Drawable;
    struct Swapchain;
    struct CommandQueue;
    struct DescriptorSet;
    struct TextureSettings;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct RenderPipelineStateDescription;

    struct DeviceShared {
        Instance instance;
        raii::Device raii;
        vk::PhysicalDevice adapter;
        uint32_t family_index;
        uint32_t queue_index;
        vk::Queue raw_queue;
        VmaAllocator allocator;

        explicit DeviceShared(Instance instance, raii::Device raii, vk::PhysicalDevice adapter, uint32_t family_index, uint32_t queue_index, vk::Queue raw_queue, VmaAllocator allocator);

        ~DeviceShared();
    };

    struct Device final {
        std::shared_ptr<DeviceShared> shared;

        explicit Device();
        explicit Device(std::shared_ptr<DeviceShared> shared);

        auto handle() -> vk::Device;
        auto dispatcher() -> const vk::raii::DeviceDispatcher&;
        auto allocator() -> VmaAllocator;
        void waitIdle();
        auto newTexture(const TextureSettings& description) -> Texture;
        auto newSampler(const vk::SamplerCreateInfo& info) -> Sampler;
        auto newBuffer(vk::BufferUsageFlags usage, uint64_t size, VmaAllocationCreateFlags options = 0) -> Buffer;
        auto newBuffer(vk::BufferUsageFlags usage, const void* pointer, uint64_t size, VmaAllocationCreateFlags options = 0) -> Buffer;
        auto newLibrary(const std::vector<char>& bytes) -> Library;
        auto newRenderPipelineState(const RenderPipelineStateDescription& description) -> RenderPipelineState;
        auto newComputePipelineState(const Function& function) -> ComputePipelineState;
        auto newCommandQueue() -> CommandQueue;
        auto newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> DescriptorSet;
        auto createSwapchain(Surface const& surface) -> Swapchain;
    };
}