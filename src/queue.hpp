#pragma once

#include "types.hpp"

#include <map>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct ClearColor {
        f32 red   = 0.0f;
        f32 greed = 0.0f;
        f32 blue  = 0.0f;
        f32 alpha = 0.0f;
    };

    struct Texture;
    struct RenderingColorAttachmentInfo {
        Arc<Texture>            texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        Arc<Texture>            resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        ClearColor              clearColor         = {};
    };

    struct RenderingDepthAttachmentInfo {
        Arc<Texture>            texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        Arc<Texture>            resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        f32                     clearDepth         = {};
    };

    struct RenderingStencilAttachmentInfo {
        Arc<Texture>            texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        Arc<Texture>            resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        u32                     clearStencil       = {};
    };

    struct RenderingColorAttachmentInfoArray {
        std::vector<RenderingColorAttachmentInfo> elements{};

        auto operator[](size_t i) -> RenderingColorAttachmentInfo& {
            if (elements.size() >= i) {
                elements.resize(i + 1, RenderingColorAttachmentInfo{});
            }
            return elements[i];
        }
    };

    struct RenderingInfo {
        vk::Rect2D                        renderArea        = {};
        u32                               layerCount        = {};
        u32                               viewMask          = {};
        RenderingColorAttachmentInfoArray colorAttachments  = {};
        RenderingDepthAttachmentInfo      depthAttachment   = {};
        RenderingStencilAttachmentInfo    stencilAttachment = {};
    };

    struct Context;
    struct Drawable;
    struct CommandQueue;
    struct PipelineState;
    struct CommandBuffer final {
        friend Context;
        friend CommandQueue;

    private:
//        vk::RenderPass renderPass = {};
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
//        auto makePipeline(i32 subpass) -> vk::Pipeline;
        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const RenderingColorAttachmentInfo& in);
        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const RenderingDepthAttachmentInfo& in);
        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const RenderingStencilAttachmentInfo& in);

    public:
        void begin(const vk::CommandBufferBeginInfo& info);
        void end();
        void submit();
        void present(Drawable* drawable);
        void setPipelineState(const Arc<PipelineState>& state);
//        void beginRenderPass(const vk::RenderPassBeginInfo& info, vk::SubpassContents contents);
//        void endRenderPass();
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

//        // todo: move to global cache?
//        std::map<std::tuple<Arc<PipelineState>, vk::RenderPass, i32>, vk::Pipeline> pipelines{};
    };
}