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

    struct Device : public ManagedObject {
        rc<Adapter>                 adapter;
        vk::Device                  handle;
        vk::raii::DeviceDispatcher  dispatcher;
        VmaAllocator                allocator;

        explicit Device(rc<Adapter> adapter, vk::DeviceCreateInfo const& create_info);
        ~Device() override;

        void waitIdle(this Device& self);
        auto newTexture(this Device& self, const TextureDescription& description) -> rc<Texture>;
        auto newSampler(this Device& self, const vk::SamplerCreateInfo& info) -> rc<Sampler>;
        auto newBuffer(this Device& self, vk::BufferUsageFlags usage, uint64_t size, StorageMode storage, VmaAllocationCreateFlags options = 0) -> rc<Buffer>;
        auto newBuffer(this Device& self, vk::BufferUsageFlags usage, const void* pointer, uint64_t size, StorageMode storage, VmaAllocationCreateFlags options = 0) -> rc<Buffer>;
        auto newLibrary(this Device& self, std::span<char const> bytes) -> rc<Library>;
        auto newDepthStencilState(this Device& self, DepthStencilStateDescription const& description) -> rc<DepthStencilState>;
        auto newRenderPipelineState(this Device& self, RenderPipelineStateDescription const& description) -> rc<RenderPipelineState>;
        auto newComputePipelineState(this Device& self, rc<Function> const& function) -> rc<ComputePipelineState>;
        auto newCommandQueue(this Device& self) -> rc<CommandQueue>;
        auto createSwapchain(this Device& self, rc<Surface> const& surface) -> rc<Swapchain>;
    };
}