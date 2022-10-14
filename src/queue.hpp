#pragma once

#include "types.hpp"

#include <map>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Texture;
    struct RenderingAttachmentInfo {
        Arc<Texture>            texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        Arc<Texture>            resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        vk::ClearValue          clearValue         = {};
    };

    struct RenderingAttachmentInfoArray {
        std::vector<RenderingAttachmentInfo> elements{};

        auto operator[](size_t i) -> RenderingAttachmentInfo& {
            if (elements.size() >= i) {
                elements.resize(i + 1, RenderingAttachmentInfo{});
            }
            return elements[i];
        }
    };

    struct RenderingInfo {
        vk::Rect2D                   renderArea        = {};
        u32                          layerCount        = {};
        u32                          viewMask          = {};
        RenderingAttachmentInfoArray colorAttachments  = {};
        RenderingAttachmentInfo      depthAttachment   = {};
        RenderingAttachmentInfo      stencilAttachment = {};
    };

    struct Context;
    struct Drawable;
    struct CommandQueue;
    struct PipelineState;
    struct CommandBuffer final {
        friend Context;
        friend CommandQueue;

    private:
        vk::RenderPass renderPass = {};
        Arc<PipelineState> pipelineState = {};
        vk::RenderingAttachmentInfo depthAttachment = {};
        vk::RenderingAttachmentInfo stencilAttachment = {};
        std::vector<vk::RenderingAttachmentInfo> colorAttachments = {};

    public:
        Context* context{};
        CommandQueue* commandQueue{};

        vk::Fence fence{};
        vk::Semaphore semaphore{};
        vk::CommandBuffer handle{};

    private:
        void reset();
        auto makePipeline(i32 subpass) -> vk::Pipeline;
        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const RenderingAttachmentInfo& in);

    public:
        void begin(const vk::CommandBufferBeginInfo& info);
        void end();
        void submit();
        void present(Drawable* drawable);
        void setPipelineState(const Arc<PipelineState>& state);
        void beginRenderPass(const vk::RenderPassBeginInfo& info, vk::SubpassContents contents);
        void endRenderPass();
        void beginRendering(const RenderingInfo& description);
        void endRendering();
        void setScissor(u32 firstScissor, const vk::Rect2D& rect);
        void setViewport(u32 firstViewport, const vk::Viewport& viewport);
        void waitUntilCompleted();

        void draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
        void drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance);
    };

    struct CommandQueue final {
        friend Context;
        friend CommandBuffer;

    public:
        CommandQueue();
        ~CommandQueue();

    public:
        auto makeCommandBuffer() -> CommandBuffer*;

    private:
        Context* context{};

        vk::Queue handle{};
        vk::CommandPool pool{};

        std::vector<vk::Fence> fences{};
        std::vector<vk::Semaphore> semaphores{};
        std::vector<vk::CommandBuffer> rawCommandBuffers{};

        std::vector<CommandBuffer> commandBuffers{};

        // todo: move to global cache?
        std::map<std::tuple<Arc<PipelineState>, vk::RenderPass, i32>, vk::Pipeline> pipelines{};
    };
}