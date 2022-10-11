#pragma once

#include "pass.hpp"
#include "material.hpp"

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Drawable;
    struct CommandQueue;
    struct CommandBuffer final {
        friend CommandQueue;

    private:
        vk::RenderPass renderPass = {};
        Arc<PipelineState> pipelineState = {};

        // todo: move to global cache?
        std::map<std::tuple<Arc<PipelineState>, vk::RenderPass, i32>, vk::Pipeline> pipelines{};

    public:
        CommandQueue* owner{};

        vk::Fence fence{};
        vk::Semaphore semaphore{};
        vk::CommandBuffer handle{};

    private:
        void clear();
        auto makePipeline(i32 subpass) -> vk::Pipeline;

    public:
        void submit();
        void present(Drawable* drawable);
        void setPipelineState(const Arc<PipelineState>& state);
        void beginRenderPass(const vk::RenderPassBeginInfo& info, vk::SubpassContents contents);
        void endRenderPass();

        void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
        void drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance);
    };

    struct CommandQueue final {
        friend Context;
        friend CommandBuffer;

    private:
        Context* context{};

        vk::Queue queue{};
        vk::CommandPool pool{};

        std::vector<vk::Fence> fences{};
        std::vector<vk::Semaphore> semaphores{};
        std::vector<vk::CommandBuffer> handles{};

        std::vector<CommandBuffer> list{};

    public:
        auto makeCommandBuffer() -> CommandBuffer*;

    private:
        void clearCommandBuffers();
    };
}