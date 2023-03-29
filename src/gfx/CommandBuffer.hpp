#pragma once

#include "Device.hpp"
#include "Texture.hpp"
#include "ClearColor.hpp"
#include "CommandQueue.hpp"

#include <optional>

namespace gfx {
    struct Device;
    struct Buffer;
    struct Texture;
    struct Drawable;
    struct CommandBuffer;
    struct RenderPipelineState;
    struct RenderCommandEncoder;
    struct ComputePipelineState;

    struct RenderingColorAttachmentInfo {
        ManagedShared<Texture>  texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        ManagedShared<Texture>  resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        ClearColor              clearColor         = {};
    };

    struct RenderingDepthAttachmentInfo {
        ManagedShared<Texture>  texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        ManagedShared<Texture>  resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        float                   clearDepth         = {};
    };

    struct RenderingStencilAttachmentInfo {
        ManagedShared<Texture>  texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        ManagedShared<Texture>  resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        uint32_t                clearStencil       = {};
    };

    struct RenderingColorAttachmentInfoArray {
        std::vector<RenderingColorAttachmentInfo> elements = {};

        auto operator[](size_t i) -> RenderingColorAttachmentInfo& {
            if (elements.size() >= i) {
                elements.resize(i + 1);
            }
            return elements[i];
        }
    };

    struct RenderingInfo {
        uint32_t                          viewMask          = {};
        uint32_t                          layerCount        = {};
        vk::Rect2D                        renderArea        = {};
        RenderingDepthAttachmentInfo      depthAttachment   = {};
        RenderingStencilAttachmentInfo    stencilAttachment = {};
        RenderingColorAttachmentInfoArray colorAttachments  = {};
    };

    struct RenderCommandEncoder : ManagedObject<RenderCommandEncoder> {
    private:
        friend CommandBuffer;

        ManagedShared<CommandBuffer> commandBuffer;
        ManagedShared<RenderPipelineState> currentPipelineState;

    public:
        RenderCommandEncoder(const ManagedShared<CommandBuffer>& commandBuffer);

    private:
        void _beginRendering(const RenderingInfo& info);
        void _endRendering();

    public:
        auto getCommandBuffer() -> ManagedShared<CommandBuffer>;
        void endEncoding();
        void setRenderPipelineState(const ManagedShared<RenderPipelineState>& pipelineState);
        void setScissor(uint32_t firstScissor, const vk::Rect2D& rect);
        void setViewport(uint32_t firstViewport, const vk::Viewport& viewport);
        void bindIndexBuffer(const ManagedShared<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType);
        void bindVertexBuffer(int firstBinding, const ManagedShared<Buffer>& buffer, vk::DeviceSize offset);
        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
        void bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot);
        void pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);
    };

    struct ComputeCommandEncoder : ManagedObject<ComputeCommandEncoder> {
    private:
        friend CommandBuffer;

        ManagedShared<CommandBuffer> commandBuffer;
        ManagedShared<ComputePipelineState> currentPipelineState;

    public:
        ComputeCommandEncoder(const ManagedShared<CommandBuffer>& commandBuffer);

    public:
        void setComputePipelineState(const ManagedShared<ComputePipelineState>& pipelineState);
        void bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot);
        void pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    };

    struct CommandBuffer : ManagedObject<CommandBuffer> {
        ManagedShared<Device>           device              = {};
        ManagedShared<CommandQueue>     queue               = {};
        vk::CommandBuffer               raw                 = {};
        vk::Fence                       fence               = {};
        vk::Semaphore                   semaphore           = {};
        std::vector<vk::DescriptorPool> descriptor_pools    = {};

        explicit CommandBuffer(const ManagedShared<Device>& device, const ManagedShared<CommandQueue>& queue);
        ~CommandBuffer() override;

        void begin(const vk::CommandBufferBeginInfo& info);
        void end();
        void submit();
        void present(const Drawable& drawable);
        void waitUntilCompleted();
        void setImageLayout(const ManagedShared<Texture>& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask);

        auto newDescriptorSet(const ManagedShared<RenderPipelineState>& render_pipeline_state, uint32_t index) -> vk::DescriptorSet;
        auto newRenderCommandEncoder(const RenderingInfo& info) -> ManagedShared<RenderCommandEncoder>;
        auto newComputeCommandEncoder() -> ManagedShared<ComputeCommandEncoder>;
    };
}