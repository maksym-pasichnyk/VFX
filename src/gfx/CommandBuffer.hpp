#pragma once

#include "Device.hpp"
#include "ClearColor.hpp"
#include "CommandQueue.hpp"

#include <optional>

namespace gfx {
    struct Device;
    struct Buffer;
    struct Texture;
    struct Drawable;
    struct CommandBuffer;
    struct DescriptorSet;
    struct RenderPipelineState;
    struct RenderCommandEncoder;
    struct ComputePipelineState;

    struct RenderingColorAttachmentInfo {
        std::optional<Texture>  texture            = std::nullopt;
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        std::optional<Texture>  resolveTexture     = std::nullopt;
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        ClearColor              clearColor         = {};
    };

    struct RenderingDepthAttachmentInfo {
        std::optional<Texture>  texture            = std::nullopt;
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        std::optional<Texture>  resolveTexture     = std::nullopt;
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        float                 clearDepth         = {};
    };

    struct RenderingStencilAttachmentInfo {
        std::optional<Texture>  texture            = std::nullopt;
        vk::ImageLayout         imageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits resolveMode        = vk::ResolveModeFlagBits::eNone;
        std::optional<Texture>  resolveTexture     = std::nullopt;
        vk::ImageLayout         resolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    loadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   storeOp            = vk::AttachmentStoreOp::eStore;
        uint32_t                clearStencil       = {};
    };

    struct RenderingColorAttachmentInfoArray {
        std::vector<RenderingColorAttachmentInfo> elements = {};

        auto operator[](size_t i) -> RenderingColorAttachmentInfo& {
            if (elements.size() >= i) {
                elements.resize(i + 1, RenderingColorAttachmentInfo{});
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

    struct CommandBufferShared {
        Device                          device;
        CommandQueue                    queue;
        vk::CommandBuffer               raw                 = {};
        vk::Fence                       fence               = {};
        vk::Semaphore                   semaphore           = {};
        vk::Pipeline                    pipeline            = {};
        vk::PipelineLayout              pipeline_layout     = {};
        vk::PipelineBindPoint           pipeline_bind_point = {};
        std::vector<vk::DescriptorPool> descriptor_pools    = {};

        explicit CommandBufferShared(Device device, CommandQueue queue);
        ~CommandBufferShared();
    };

    struct CommandBuffer final {
        std::shared_ptr<CommandBufferShared> shared;

        explicit CommandBuffer() : shared(nullptr) {}
        explicit CommandBuffer(std::shared_ptr<CommandBufferShared> shared) : shared(std::move(shared)) {}

        void begin(const vk::CommandBufferBeginInfo& info);
        void end();
        void submit();
        void present(const Drawable& drawable);
        void setComputePipelineState(ComputePipelineState const& state);
        void bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot);
        void pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        void waitUntilCompleted();
        void setImageLayout(Texture const& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask);

        void beginRendering(const RenderingInfo& info);
        void endRendering();
        void setRenderPipelineState(const RenderPipelineState& state);
        void setScissor(uint32_t firstScissor, const vk::Rect2D& rect);
        void setViewport(uint32_t firstViewport, const vk::Viewport& viewport);
        void bindIndexBuffer(const Buffer& buffer, vk::DeviceSize offset, vk::IndexType indexType);
        void bindVertexBuffer(int firstBinding, const Buffer& buffer, vk::DeviceSize offset);
        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
        auto newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> vk::DescriptorSet;
    };
}