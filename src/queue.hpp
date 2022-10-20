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

    struct Buffer;
    struct Context;
    struct Drawable;
    struct CommandQueue;
    struct PipelineState;
    struct CommandBuffer final {
        friend Context;
        friend CommandQueue;

    private:
        bool retainedReferences = false;
//        vk::RenderPass renderPass = {};
        vk::RenderingAttachmentInfo depthAttachment = {};
        vk::RenderingAttachmentInfo stencilAttachment = {};
        std::vector<vk::RenderingAttachmentInfo> colorAttachments = {};

        std::vector<vk::MemoryBarrier2> memoryBarriers = {};
        std::vector<vk::ImageMemoryBarrier2> imageMemoryBarriers = {};
        std::vector<vk::BufferMemoryBarrier2> bufferMemoryBarriers = {};

        std::vector<Arc<Buffer>> bufferReferences{};
        std::vector<Arc<Texture>> textureReferences{};

    public:
        Context* context{};
        CommandQueue* commandQueue{};

        vk::UniqueFence fence{};
        vk::UniqueSemaphore semaphore{};
        vk::UniqueCommandBuffer handle{};

    private:
        void reset();
        void releaseReferences();
//        auto makePipeline(i32 subpass) -> vk::Pipeline;

    public:
        auto getRetainedReferences() const -> bool;
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

        void flushBarriers();
        void memoryBarrier(const vk::MemoryBarrier2& barrier);
        void imageMemoryBarrier(const vk::ImageMemoryBarrier2& barrier);
        void bufferMemoryBarrier(const vk::BufferMemoryBarrier2& barrier);

        void bindIndexBuffer(const Arc<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType);
        void bindVertexBuffer(int firstBinding, const Arc<Buffer>& buffer, vk::DeviceSize offset);
    };

    struct CommandQueue final {
        friend Context;
        friend CommandBuffer;

    public:
        CommandQueue();
        ~CommandQueue();

    public:
        auto makeCommandBuffer() -> CommandBuffer*;
        auto makeCommandBufferWithUnretainedReferences() -> CommandBuffer*;

    private:
        Context* context{};

        vk::Queue handle{};
        vk::CommandPool pool{};
        std::vector<Arc<CommandBuffer>> retainedList{};
        std::vector<Arc<CommandBuffer>> unretainedList{};
    };
}