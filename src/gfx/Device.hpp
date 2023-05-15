#pragma once

#include "Instance.hpp"
#include "ManagedObject.hpp"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#endif

#include <vk_mem_alloc.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

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
    struct TextureDescription;
    struct DepthStencilState;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct DepthStencilStateDescription;
    struct RenderPipelineStateDescription;

    enum class StorageMode {
        ePrivate,
        eManaged,
        eShared,
        eLazy,
    };

    struct Device : ManagedObject<Device> {
        ManagedShared<Instance> instance;
        raii::Device            raii;
        vk::PhysicalDevice      adapter;
        uint32_t                family_index;
        uint32_t                queue_index;
        vk::Queue               queue;
        VmaAllocator            allocator;

        explicit Device(ManagedShared<Instance> instance, raii::Device raii, vk::PhysicalDevice adapter, uint32_t family_index, uint32_t queue_index, vk::Queue raw_queue, VmaAllocator allocator);
        ~Device() override;

        void waitIdle();
        auto newTexture(const TextureDescription& description) -> ManagedShared<Texture>;
        auto newSampler(const vk::SamplerCreateInfo& info) -> ManagedShared<Sampler>;
        auto newBuffer(vk::BufferUsageFlags usage, uint64_t size, StorageMode storage, VmaAllocationCreateFlags options = 0) -> ManagedShared<Buffer>;
        auto newBuffer(vk::BufferUsageFlags usage, const void* pointer, uint64_t size, StorageMode storage, VmaAllocationCreateFlags options = 0) -> ManagedShared<Buffer>;
        auto newLibrary(const std::vector<char>& bytes) -> ManagedShared<Library>;
        auto newDepthStencilState(DepthStencilStateDescription const& description) -> ManagedShared<DepthStencilState>;
        auto newRenderPipelineState(RenderPipelineStateDescription const& description) -> ManagedShared<RenderPipelineState>;
        auto newComputePipelineState(ManagedShared<Function> const& function) -> ManagedShared<ComputePipelineState>;
        auto newCommandQueue() -> ManagedShared<CommandQueue>;
        auto createSwapchain(ManagedShared<Surface> const& surface) -> ManagedShared<Swapchain>;
    };
}