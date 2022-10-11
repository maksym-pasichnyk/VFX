#pragma once

#include <list>
#include <glm/vec4.hpp>
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_beta.h>

#include "pass.hpp"
#include "mesh.hpp"
#include "queue.hpp"
#include "types.hpp"
#include "buffer.hpp"
#include "display.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "spirv_reflect.h"

namespace vfx {
    struct ContextDescription {
        std::string app_name = {};
        vk::InstanceCreateFlags flags{};
        std::vector<const char*> layers{};
        std::vector<const char*> extensions{};

        bool enable_debug = false;
    };

    struct Context {
    public:
        static constexpr auto MAX_FRAMES_IN_FLIGHT = 3u;

        vk::DynamicLoader dl{};
        vk::Instance instance{};
        VmaAllocator allocator{};
        vk::DebugUtilsMessengerEXT debug_utils{};

        u32 present_family{};
        u32 graphics_family{};
        vk::Queue present_queue{};
        vk::Queue graphics_queue{};
        vk::PhysicalDevice physical_device{};

        vk::Device logical_device{};
        vk::Format depth_format{};

    public:
        explicit Context(const ContextDescription& description);
        ~Context();

    private:
        void create_instance(const ContextDescription& description);
        void select_physical_device();
        void create_logical_device();
        void create_memory_allocator();
        auto create_shader_module(std::span<const char> bytes) const -> vk::ShaderModule;

    public:
        auto makeRenderPass(const RenderPassDescription& description) -> Arc<RenderPass>;
        void freeRenderPass(const Arc<RenderPass>& pass);

        auto makeTexture(const TextureDescription& description) -> Arc<Texture>;
        void freeTexture(const Arc<Texture>& texture);

        auto makeBuffer(BufferUsage target, u64 size) -> Arc<Buffer>;
        void freeBuffer(const Arc<Buffer>& buffer);

        auto makeMesh() -> Arc<Mesh>;
        void freeMesh(const Arc<Mesh>& mesh);

        auto makePipelineState(const PipelineStateDescription& description) -> Arc<PipelineState>;
        void freePipelineState(const Arc<PipelineState>& pipelineState);

        auto makeCommandQueue(u32 count) -> Arc<CommandQueue>;
        void freeCommandQueue(const Arc<CommandQueue>& queue);
    };
}