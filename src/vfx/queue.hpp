#pragma once

#include "types.hpp"

#include <map>
#include <set>
#include <tuple>
#include <vector>
#include <concepts>
#include <vulkan/vulkan.hpp>

namespace vfx {
    struct ClearColor {
        f32 red   = 0.0f;
        f32 greed = 0.0f;
        f32 blue  = 0.0f;
        f32 alpha = 0.0f;

        static auto init(f32 r, f32 g, f32 b, f32 a) -> ClearColor {
            return { r, g, b, a };
        }

        static auto init(i32 r, i32 g, i32 b, i32 a) -> ClearColor {
            return {
                std::clamp(f32(r) / 255.f, 0.f, 1.f),
                std::clamp(f32(g) / 255.f, 0.f, 1.f),
                std::clamp(f32(b) / 255.f, 0.f, 1.f),
                std::clamp(f32(a) / 255.f, 0.f, 1.f)
            };
        }
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
    struct Device;
    struct Drawable;
    struct CommandQueue;
    struct ResourceGroup;
    struct RenderPipelineState;
    struct ComputePipelineState;
    struct CommandBuffer final {
        friend Device;
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

        vk::Pipeline currentPipeline = {};
        vk::PipelineLayout currentPipelineLayout = {};
        vk::PipelineBindPoint currentPipelineBindPoint = {};

        std::set<Arc<Buffer>> bufferReferences{};
        std::set<Arc<Texture>> textureReferences{};
        std::set<Arc<ResourceGroup>> resourceGroupReferences{};

    public:
        Device* device{};
        CommandQueue* commandQueue{};

        vk::UniqueFence fence{};
        vk::UniqueSemaphore semaphore{};
        vk::UniqueCommandBuffer handle{};

    private:
        void reset();
        void releaseReferences();

        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const vfx::RenderingColorAttachmentInfo& in);
        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const vfx::RenderingDepthAttachmentInfo& in);
        void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const vfx::RenderingStencilAttachmentInfo& in);
//        auto makePipeline(i32 subpass) -> vk::Pipeline;

    public:
        auto getRetainedReferences() const -> bool;
        void begin(const vk::CommandBufferBeginInfo& info);
        void end();
        void submit();
        void present(Drawable* drawable);
        void setRenderPipelineState(const Arc<RenderPipelineState>& state);
        void setComputePipelineState(const Arc<ComputePipelineState>& state);
        void bindResourceGroup(const Arc<ResourceGroup>& group, u32 index);
        void pushConstants(const Arc<RenderPipelineState>& state, vk::ShaderStageFlags stageFlags, u32 offset, u32 size, const void* data);
//        void beginRenderPass(const vk::RenderPassBeginInfo& info, vk::SubpassContents contents);
//        void endRenderPass();
        void beginRendering(const RenderingInfo& description);
        void endRendering();
        void dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ);
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
        friend Device;
        friend CommandBuffer;

    public:
        CommandQueue();
        ~CommandQueue();

    public:
        auto makeCommandBuffer() -> CommandBuffer*;
        auto makeCommandBufferWithUnretainedReferences() -> CommandBuffer*;

    private:
        Device* device{};

        vk::Queue handle{};
        vk::CommandPool pool{};
        std::vector<Arc<CommandBuffer>> retainedList{};
        std::vector<Arc<CommandBuffer>> unretainedList{};
    };
}