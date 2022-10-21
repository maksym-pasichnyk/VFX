#pragma once

#include "types.hpp"
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Buffer;
    struct Texture;
    struct Context;
    struct Sampler;
    struct Library;
    struct Surface;
    struct Function;
    struct Drawable;
    struct RenderPass;
    struct CommandQueue;
    struct PipelineState;
    struct ResourceGroup;
    struct TextureDescription;
    struct RenderPassDescription;
    struct PipelineStateDescription;

    struct Device {
    public:
        VmaAllocator allocator{};

        vk::PhysicalDevice gpu{};
        vk::UniqueDevice handle{};

        vk::Queue graphics_queue{};
        u32 graphics_queue_family_index{};

        vk::Queue present_queue{};
        u32 present_queue_family_index{};

        vk::Queue compute_queue{};
        u32 compute_queue_family_index{};

    public:
        explicit Device(const Arc<Context>& context);
        ~Device();

    private:
        void select_physical_device(const Arc<Context>& context);
        void create_logical_device(const Arc<Context>& context);
        void create_memory_allocator(const Arc<Context>& context);

    public:
        auto makeRenderPass(const RenderPassDescription& description) -> Arc<RenderPass>;
        void freeRenderPass(RenderPass* pass);

        auto makeTexture(const TextureDescription& description) -> Arc<Texture>;
        void freeTexture(Texture* texture);

        auto makeSampler(const vk::SamplerCreateInfo& info) -> Arc<Sampler>;
        void freeSampler(Sampler* sampler);

        auto makeBuffer(vk::BufferUsageFlags usage, u64 size, VmaAllocationCreateFlags options = 0) -> Arc<Buffer>;
        auto makeBuffer(vk::BufferUsageFlags usage, u64 size, const void* data, VmaAllocationCreateFlags options = 0) -> Arc<Buffer>;
        void freeBuffer(Buffer* buffer);

        auto makeLibrary(const std::vector<char>& bytes) -> Arc<Library>;
        void freeLibrary(Library* library);

        auto makePipelineState(const PipelineStateDescription& description) -> Arc<PipelineState>;
        void freePipelineState(PipelineState* pipelineState);

        auto makeCommandQueue() -> Arc<CommandQueue>;
        void freeCommandQueue(CommandQueue* queue);

        auto makeResourceGroup(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> Arc<ResourceGroup>;
        void freeResourceGroup(ResourceGroup* group);
    };
}