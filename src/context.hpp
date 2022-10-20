#pragma once

#include "types.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

namespace vfx {
    struct ContextDescription {
        std::string app_name = {};
        vk::InstanceCreateFlags flags{};
        std::vector<const char*> extensions{};

        bool enableDebugUtils = false;
    };

    enum class BufferUsage;

    struct Mesh;
    struct Buffer;
    struct Texture;
    struct Sampler;
    struct Library;
    struct Function;
    struct RenderPass;
    struct CommandQueue;
    struct PipelineState;
    struct TextureDescription;
    struct RenderPassDescription;
    struct PipelineStateDescription;
    struct Context {
    public:
        vk::DynamicLoader dl{};
        vk::UniqueInstance instance{};
        vk::UniqueDebugUtilsMessengerEXT debug_utils{};

        VmaAllocator allocator{};

        vk::PhysicalDevice gpu{};

        vk::UniqueDevice device{};

        vk::Queue graphics_queue{};
        u32 graphics_queue_family_index{};

        vk::Queue present_queue{};
        u32 present_queue_family_index{};

        vk::Queue compute_queue{};
        u32 compute_queue_family_index{};

    public:
        explicit Context(const ContextDescription& description);
        ~Context();

    private:
        void create_instance(const ContextDescription& description);
        void select_physical_device();
        void create_logical_device();
        void create_memory_allocator();

    public:
        auto makeRenderPass(const RenderPassDescription& description) -> Arc<RenderPass>;
        void freeRenderPass(RenderPass* pass);

        auto makeTexture(const TextureDescription& description) -> Arc<Texture>;
        void freeTexture(Texture* texture);

        auto makeSampler(const vk::SamplerCreateInfo& info) -> Arc<Sampler>;
        void freeSampler(Sampler* sampler);

        auto makeBuffer(BufferUsage target, u64 size) -> Arc<Buffer>;
        void freeBuffer(Buffer* buffer);

        auto makeLibrary(const std::vector<char>& bytes) -> Arc<Library>;
        void freeLibrary(Library* library);

        auto makePipelineState(const PipelineStateDescription& description) -> Arc<PipelineState>;
        void freePipelineState(PipelineState* pipelineState);

        auto makeCommandQueue() -> Arc<CommandQueue>;
        void freeCommandQueue(CommandQueue* queue);
    };

    extern auto createSystemDefaultContext() -> Arc<Context>;
}