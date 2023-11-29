#pragma once

#include "Device.hpp"
#include "Texture.hpp"
#include "ClearColor.hpp"
#include "CommandQueue.hpp"
#include "RenderPipelineState.hpp"

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
        rc<Texture>  texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        rc<Texture>  resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        ClearColor              clearColor         = {};
    };

    struct RenderingDepthAttachmentInfo {
        rc<Texture>             texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        rc<Texture>             resolveTexture     = {};
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        float                   clearDepth         = {};
    };

    struct RenderingStencilAttachmentInfo {
        rc<Texture>             texture            = {};
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        rc<Texture>             resolveTexture     = {};
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

    struct RenderCommandEncoder : public ManagedObject {
        enum : uint32_t {
            RenderCommandEncoderPipeline    = 1 << 0
        };

        uint32_t                            flags_                      = {};
        rc<CommandBuffer>                   commandBuffer               = {};
        rc<DepthStencilState>               depthStencilState_          = {};
        rc<RenderPipelineState>             renderPipelineState_        = {};
        bool                                depthClampEnable_           = {};
        bool                                rasterizerDiscardEnable_    = {};
        vk::PolygonMode                     polygonMode_                = {};
        float                               lineWidth_                  = {};
        vk::CullModeFlagBits                cullMode_                   = {};
        vk::FrontFace                       frontFace_                  = {};
        bool                                depthBiasEnable_            = {};
        float                               depthBiasConstantFactor_    = {};
        float                               depthBiasClamp_             = {};
        float                               depthBiasSlopeFactor_       = {};

        RenderCommandEncoder(const rc<CommandBuffer>& commandBuffer);

        void _beginRendering(const RenderingInfo& info);
        void _endRendering();
        void _setup();

        auto getCommandBuffer() -> rc<CommandBuffer>;
        void endEncoding();
        void setDepthClampEnable(bool depthClampEnable);
        void setRasterizerDiscardEnable(bool rasterizerDiscardEnable);
        void setPolygonMode(vk::PolygonMode polygonMode);
        void setLineWidth(float lineWidth);
        void setCullMode(vk::CullModeFlagBits cullMode);
        void setFrontFace(vk::FrontFace frontFace);
        void setDepthBiasEnable(bool depthBiasEnable);
        void setDepthBiasConstantFactor(float depthBiasConstantFactor);
        void setDepthBiasClamp(float depthBiasClamp);
        void setDepthBiasSlopeFactor(float depthBiasSlopeFactor);
        void setDepthStencilState(rc<DepthStencilState> depthStencilState);
        void setRenderPipelineState(rc<RenderPipelineState> renderPipelineState);
        void setScissor(uint32_t firstScissor, const vk::Rect2D& rect);
        void setViewport(uint32_t firstViewport, const vk::Viewport& viewport);
        void bindIndexBuffer(const rc<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType);
        void bindVertexBuffer(int firstBinding, const rc<Buffer>& buffer, vk::DeviceSize offset);
        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
        void bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot);
        void pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);
    };

    struct ComputeCommandEncoder : public ManagedObject {
        rc<CommandBuffer>        commandBuffer;
        rc<ComputePipelineState> currentPipelineState;

        ComputeCommandEncoder(const rc<CommandBuffer>& commandBuffer);

        void setComputePipelineState(const rc<ComputePipelineState>& pipelineState);
        void bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot);
        void pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    };

    struct CommandBuffer : public ManagedObject {
        rc<Device>                      device              = {};
        rc<CommandQueue>                queue               = {};
        vk::CommandBuffer               handle              = {};
        vk::Fence                       fence               = {};
        vk::Semaphore                   semaphore           = {};
        std::vector<vk::DescriptorPool> descriptor_pools    = {};

        explicit CommandBuffer(const rc<Device>& device, const rc<CommandQueue>& queue);
        ~CommandBuffer() override;

        void begin(const vk::CommandBufferBeginInfo& begin_info);
        void end();
        void submit();
        void present(const Drawable& drawable);
        void waitUntilCompleted();
        void setImageLayout(const rc<Texture>& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask);

        auto newDescriptorSet(const rc<RenderPipelineState>& render_pipeline_state, uint32_t index) -> vk::DescriptorSet;
        auto newRenderCommandEncoder(const RenderingInfo& info) -> rc<RenderCommandEncoder>;
        auto newComputeCommandEncoder() -> rc<ComputeCommandEncoder>;
    };
}