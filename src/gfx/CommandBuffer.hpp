#pragma once

#include "Object.hpp"

#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Buffer;
    struct Texture;
    struct Drawable;
    struct CommandQueue;
    struct CommandBuffer;
    struct DescriptorSet;
    struct RenderPipelineState;
    struct RenderCommandEncoder;
    struct ComputePipelineState;

    struct ClearColor {
        float_t red   = 0.0f;
        float_t greed = 0.0f;
        float_t blue  = 0.0f;
        float_t alpha = 0.0f;

        static auto init(float_t r, float_t g, float_t b, float_t a) -> ClearColor {
            return { r, g, b, a };
        }

        static auto init(int32_t r, int32_t g, int32_t b, int32_t a) -> ClearColor {
            return {
                std::clamp(float_t(r) / 255.f, 0.f, 1.f),
                std::clamp(float_t(g) / 255.f, 0.f, 1.f),
                std::clamp(float_t(b) / 255.f, 0.f, 1.f),
                std::clamp(float_t(a) / 255.f, 0.f, 1.f)
            };
        }
    };

    struct RenderingColorAttachmentInfo {
        friend CommandBuffer;

    private:
        SharedPtr<Texture>      mTexture            = {};
        vk::ImageLayout         mImageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits mResolveMode        = vk::ResolveModeFlagBits::eNone;
        SharedPtr<Texture>      mResolveTexture     = {};
        vk::ImageLayout         mResolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    mLoadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   mStoreOp            = vk::AttachmentStoreOp::eStore;
        ClearColor              mClearColor         = {};

    public:
        void setTexture(SharedPtr<Texture> texture) {
            mTexture = std::move(texture);
        }
        void setImageLayout(vk::ImageLayout imageLayout) {
            mImageLayout = imageLayout;
        }
        void setResolveMode(vk::ResolveModeFlagBits resolveMode) {
            mResolveMode = resolveMode;
        }
        void setResolveTexture(SharedPtr<Texture> resolveTexture) {
            mResolveTexture = std::move(resolveTexture);
        }
        void setResolveImageLayout(vk::ImageLayout resolveImageLayout) {
            mResolveImageLayout = resolveImageLayout;
        }
        void setLoadOp(vk::AttachmentLoadOp loadOp) {
            mLoadOp = loadOp;
        }
        void setStoreOp(vk::AttachmentStoreOp storeOp) {
            mStoreOp = storeOp;
        }
        void setClearColor(ClearColor clearColor) {
            mClearColor = clearColor;
        }
    };

    struct RenderingDepthAttachmentInfo {
        friend CommandBuffer;

    private:
        SharedPtr<Texture>      mTexture            = {};
        vk::ImageLayout         mImageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits mResolveMode        = vk::ResolveModeFlagBits::eNone;
        SharedPtr<Texture>      mResolveTexture     = {};
        vk::ImageLayout         mResolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    mLoadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   mStoreOp            = vk::AttachmentStoreOp::eStore;
        float_t                 mClearDepth         = {};

    public:
        void setTexture(SharedPtr<Texture> texture) {
            mTexture = std::move(texture);
        }
        void setImageLayout(vk::ImageLayout imageLayout) {
            mImageLayout = imageLayout;
        }
        void setResolveMode(vk::ResolveModeFlagBits resolveMode) {
            mResolveMode = resolveMode;
        }
        void setResolveTexture(SharedPtr<Texture> resolveTexture) {
            mResolveTexture = std::move(resolveTexture);
        }
        void setResolveImageLayout(vk::ImageLayout resolveImageLayout) {
            mResolveImageLayout = resolveImageLayout;
        }
        void setLoadOp(vk::AttachmentLoadOp loadOp) {
            mLoadOp = loadOp;
        }
        void setStoreOp(vk::AttachmentStoreOp storeOp) {
            mStoreOp = storeOp;
        }
        void setClearDepth(float_t clearDepth) {
            mClearDepth = clearDepth;
        }
    };

    struct RenderingStencilAttachmentInfo {
        friend CommandBuffer;

    private:
        SharedPtr<Texture>      mTexture             = {};
        vk::ImageLayout         mImageLayout        = vk::ImageLayout::eUndefined;
        vk::ResolveModeFlagBits mResolveMode        = vk::ResolveModeFlagBits::eNone;
        SharedPtr<Texture>      mResolveTexture      = {};
        vk::ImageLayout         mResolveImageLayout = vk::ImageLayout::eUndefined;
        vk::AttachmentLoadOp    mLoadOp             = vk::AttachmentLoadOp::eLoad;
        vk::AttachmentStoreOp   mStoreOp            = vk::AttachmentStoreOp::eStore;
        uint32_t                mClearStencil        = {};

    public:
        void setTexture(SharedPtr<Texture> texture) {
            mTexture = std::move(texture);
        }
        void setImageLayout(vk::ImageLayout imageLayout) {
            mImageLayout = imageLayout;
        }
        void setResolveMode(vk::ResolveModeFlagBits resolveMode) {
            mResolveMode = resolveMode;
        }
        void setResolveTexture(SharedPtr<Texture> resolveTexture) {
            mResolveTexture = std::move(resolveTexture);
        }
        void setResolveImageLayout(vk::ImageLayout resolveImageLayout) {
            mResolveImageLayout = resolveImageLayout;
        }
        void setLoadOp(vk::AttachmentLoadOp loadOp) {
            mLoadOp = loadOp;
        }
        void setStoreOp(vk::AttachmentStoreOp storeOp) {
            mStoreOp = storeOp;
        }
        void setClearDepth(uint32_t clearStencil) {
            mClearStencil = clearStencil;
        }
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
        friend CommandBuffer;
        friend RenderCommandEncoder;

    private:
        uint32_t mViewMask = {};
        uint32_t mLayerCount = {};
        vk::Rect2D mRenderArea = {};
        RenderingDepthAttachmentInfo mDepthAttachment = {};
        RenderingStencilAttachmentInfo mStencilAttachment = {};
        RenderingColorAttachmentInfoArray mColorAttachments = {};

    public:
        void setRenderArea(vk::Rect2D renderArea) {
            mRenderArea = renderArea;
        }
        void setLayerCount(uint32_t layerCount) {
            mLayerCount = layerCount;
        }
        void setViewMask(uint32_t viewMask) {
            mViewMask = viewMask;
        }
        auto depthAttachment() -> RenderingDepthAttachmentInfo& {
            return mDepthAttachment;
        }
        auto stencilAttachment() -> RenderingStencilAttachmentInfo& {
            return mStencilAttachment;
        }
        auto colorAttachments() -> RenderingColorAttachmentInfoArray& {
            return mColorAttachments;
        }
    };

    struct CommandBuffer final : Referencing {
        friend Device;
        friend Texture;
        friend CommandQueue;
        friend RenderCommandEncoder;

    private:
        explicit CommandBuffer(SharedPtr<Device> device, CommandQueue* pCommandQueue, bool mRetainedReferences);
        ~CommandBuffer() override;

    private:
        bool mRetainedReferences = false;

        SharedPtr<Device> mDevice;
        CommandQueue* pCommandQueue;

        vk::Fence mFence = {};
        vk::Semaphore mSemaphore = {};
        vk::CommandBuffer mCommandBuffer = {};

        vk::Pipeline mPipeline = {};
        vk::PipelineLayout mPipelineLayout = {};
        vk::PipelineBindPoint mPipelineBindPoint = {};

    public:
        auto getRetainedReferences() -> bool;
        void begin(const vk::CommandBufferBeginInfo& info);
        void end();
        void submit();
        void present(const SharedPtr<Drawable>& drawable);
        void setComputePipelineState(const SharedPtr<ComputePipelineState>& state);
        void bindDescriptorSet(const SharedPtr<DescriptorSet>& descriptorSet, uint32_t slot);
        void pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data);
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        void waitUntilCompleted();
        void changeTextureLayout(const SharedPtr<Texture>& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask);

#pragma region RenderCommandEncoder
        void beginRendering(const RenderingInfo& info);
        void endRendering();
        void setRenderPipelineState(const SharedPtr<RenderPipelineState>& state);
        void setScissor(uint32_t firstScissor, const vk::Rect2D& rect);
        void setViewport(uint32_t firstViewport, const vk::Viewport& viewport);
        void bindIndexBuffer(const SharedPtr<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType);
        void bindVertexBuffer(int firstBinding, const SharedPtr<Buffer>& buffer, vk::DeviceSize offset);
        void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
        void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
#pragma endregion RenderCommandEncoder

    };

    struct RenderCommandEncoder final : Referencing {
    };
}