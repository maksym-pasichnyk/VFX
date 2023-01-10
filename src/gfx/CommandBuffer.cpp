#include "Swapchain.hpp"
#include "Buffer.hpp"
#include "Device.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

gfx::CommandBuffer::CommandBuffer(SharedPtr<Device> device, CommandQueue* pCommandQueue, bool mRetainedReferences)
: mDevice(std::move(device))
, pCommandQueue(pCommandQueue)
, mRetainedReferences(mRetainedReferences) {
    vk::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.setCommandPool(pCommandQueue->mCommandPool);
    allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
    allocate_info.setCommandBufferCount(1);

    vk::resultCheck(mDevice->mDevice.allocateCommandBuffers(&allocate_info, &mCommandBuffer, mDevice->mDispatchLoaderDynamic), "allocateCommandBuffers");

    vk::FenceCreateInfo fence_create_info = {};
    mFence = mDevice->mDevice.createFence(fence_create_info, nullptr, mDevice->mDispatchLoaderDynamic);

    vk::SemaphoreCreateInfo semaphore_create_info = {};
    mSemaphore = mDevice->mDevice.createSemaphore(semaphore_create_info, nullptr, mDevice->mDispatchLoaderDynamic);
}

gfx::CommandBuffer::~CommandBuffer() {
    mDevice->mDevice.destroySemaphore(mSemaphore, nullptr, mDevice->mDispatchLoaderDynamic);
    mDevice->mDevice.destroyFence(mFence, nullptr, mDevice->mDispatchLoaderDynamic);
}

auto gfx::CommandBuffer::getRetainedReferences() -> bool {
    return mRetainedReferences;
}

void gfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    mCommandBuffer.begin(info, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::end() {
    mCommandBuffer.end(mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::submit() {
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&mCommandBuffer);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&mSemaphore);

    vk::resultCheck(mDevice->mDevice.resetFences(1, &mFence, mDevice->mDispatchLoaderDynamic), "Submit");
    pCommandQueue->mGraphicsQueue.submit(submit_info, mFence, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::present(const SharedPtr<gfx::Drawable>& drawable) {
    vk::PresentInfoKHR present_info = {};
    present_info.setWaitSemaphores(mSemaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable->pLayer->mSwapchain);
    present_info.setPImageIndices(&drawable->mDrawableIndex);

    vk::Result result = pCommandQueue->mPresentQueue.presentKHR(present_info, mDevice->mDispatchLoaderDynamic);

    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void gfx::CommandBuffer::setComputePipelineState(const SharedPtr<ComputePipelineState>& state) {
    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, state->mPipeline, mDevice->mDispatchLoaderDynamic);

    mPipeline = state->mPipeline;
    mPipelineLayout = state->mPipelineLayout;
    mPipelineBindPoint = vk::PipelineBindPoint::eCompute;
}

void gfx::CommandBuffer::bindDescriptorSet(const SharedPtr<DescriptorSet>& descriptorSet, uint32_t slot) {
    mCommandBuffer.bindDescriptorSets(mPipelineBindPoint, mPipelineLayout, slot, 1, &descriptorSet->mDescriptorSet, 0, nullptr, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    mCommandBuffer.pushConstants(mPipelineLayout, stageFlags, offset, size, data, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    mCommandBuffer.dispatch(groupCountX, groupCountY, groupCountZ, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::waitUntilCompleted() {
    vk::Result result = pCommandQueue->mDevice->mDevice.waitForFences(mFence, VK_TRUE, std::numeric_limits<uint64_t>::max(), mDevice->mDispatchLoaderDynamic);
    vk::resultCheck(result, "waitForFences");
}

void gfx::CommandBuffer::changeTextureLayout(const SharedPtr<Texture>& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask) {
    vk::ImageMemoryBarrier2 barrier = {};
    barrier.setSrcStageMask(srcStageMask);
    barrier.setSrcAccessMask(srcAccessMask);
    barrier.setDstStageMask(dstStageMask);
    barrier.setDstAccessMask(dstAccessMask);
    barrier.setOldLayout(oldLayout);
    barrier.setNewLayout(newLayout);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setImage(texture->mImage);
    barrier.setSubresourceRange(texture->mImageSubresourceRange);

    vk::DependencyInfo dependency_info = {};
    dependency_info.setImageMemoryBarrierCount(1);
    dependency_info.setPImageMemoryBarriers(&barrier);

    mCommandBuffer.pipelineBarrier2(dependency_info, mDevice->mDispatchLoaderDynamic);
}

#pragma region RenderCommandEncoder
void gfx::CommandBuffer::beginRendering(const RenderingInfo& info) {
    vk::RenderingAttachmentInfo depthAttachment = {};
    vk::RenderingAttachmentInfo stencilAttachment = {};
    std::vector<vk::RenderingAttachmentInfo> colorAttachments = {};

    colorAttachments.resize(info.mColorAttachments.elements.size());
    for (uint64_t i = 0; i < info.mColorAttachments.elements.size(); ++i) {
        auto rgba = std::array{
            info.mColorAttachments.elements[i].mClearColor.red,
            info.mColorAttachments.elements[i].mClearColor.greed,
            info.mColorAttachments.elements[i].mClearColor.blue,
            info.mColorAttachments.elements[i].mClearColor.alpha
        };

        vk::ClearColorValue color = {};
        color.setFloat32(rgba);

        if (info.mColorAttachments.elements[i].mTexture) {
            colorAttachments[i].setImageView(info.mColorAttachments.elements[i].mTexture->mImageView);
            colorAttachments[i].setImageLayout(info.mColorAttachments.elements[i].mImageLayout);
        }
        if (info.mColorAttachments.elements[i].mResolveTexture) {
            colorAttachments[i].setResolveMode(info.mColorAttachments.elements[i].mResolveMode);
            colorAttachments[i].setResolveImageView(info.mColorAttachments.elements[i].mResolveTexture->mImageView);
            colorAttachments[i].setResolveImageLayout(info.mColorAttachments.elements[i].mResolveImageLayout);
        }
        colorAttachments[i].setLoadOp(info.mColorAttachments.elements[i].mLoadOp);
        colorAttachments[i].setStoreOp(info.mColorAttachments.elements[i].mStoreOp);
        colorAttachments[i].clearValue.setColor(color);
    }

    vk::RenderingInfo rendering_info = {};
    rendering_info.setRenderArea(info.mRenderArea);
    rendering_info.setLayerCount(info.mLayerCount);
    rendering_info.setViewMask(info.mViewMask);
    rendering_info.setColorAttachments(colorAttachments);

    if (info.mDepthAttachment.mTexture || info.mDepthAttachment.mResolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setDepth(info.mDepthAttachment.mClearDepth);

        if (info.mDepthAttachment.mTexture) {
            depthAttachment.setImageView(info.mDepthAttachment.mTexture->mImageView);
            depthAttachment.setImageLayout(info.mDepthAttachment.mImageLayout);
        }
        if (info.mDepthAttachment.mResolveTexture) {
            depthAttachment.setResolveMode(info.mDepthAttachment.mResolveMode);
            depthAttachment.setResolveImageView(info.mDepthAttachment.mResolveTexture->mImageView);
            depthAttachment.setResolveImageLayout(info.mDepthAttachment.mResolveImageLayout);
        }
        depthAttachment.setLoadOp(info.mDepthAttachment.mLoadOp);
        depthAttachment.setStoreOp(info.mDepthAttachment.mStoreOp);
        depthAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPDepthAttachment(&depthAttachment);
    }

    if (info.mStencilAttachment.mTexture || info.mStencilAttachment.mResolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setDepth(info.mStencilAttachment.mClearStencil);

        if (info.mStencilAttachment.mTexture) {
            stencilAttachment.setImageView(info.mStencilAttachment.mTexture->mImageView);
            stencilAttachment.setImageLayout(info.mStencilAttachment.mImageLayout);
        }
        if (info.mStencilAttachment.mResolveTexture) {
            stencilAttachment.setResolveMode(info.mStencilAttachment.mResolveMode);
            stencilAttachment.setResolveImageView(info.mStencilAttachment.mResolveTexture->mImageView);
            stencilAttachment.setResolveImageLayout(info.mStencilAttachment.mResolveImageLayout);
        }
        stencilAttachment.setLoadOp(info.mStencilAttachment.mLoadOp);
        stencilAttachment.setStoreOp(info.mStencilAttachment.mStoreOp);
        stencilAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPStencilAttachment(&stencilAttachment);
    }

    mCommandBuffer.beginRendering(rendering_info, mDevice->mDispatchLoaderDynamic);

}

void gfx::CommandBuffer::endRendering() {
    mCommandBuffer.endRendering(mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::setRenderPipelineState(const SharedPtr<RenderPipelineState>& state) {
    mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, state->mPipeline, mDevice->mDispatchLoaderDynamic);

    mPipeline = state->mPipeline;
    mPipelineLayout = state->mPipelineLayout;
    mPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
}

void gfx::CommandBuffer::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    mCommandBuffer.setScissor(firstScissor, 1, &rect, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    mCommandBuffer.setViewport(firstViewport, 1, &viewport, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::bindIndexBuffer(const SharedPtr<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    mCommandBuffer.bindIndexBuffer(buffer->mBuffer, offset, indexType, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::bindVertexBuffer(int firstBinding, const SharedPtr<Buffer>& buffer, vk::DeviceSize offset) {
    mCommandBuffer.bindVertexBuffers(firstBinding, 1, &buffer->mBuffer, &offset, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    mCommandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance, mDevice->mDispatchLoaderDynamic);
}

void gfx::CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    mCommandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, mDevice->mDispatchLoaderDynamic);
}
#pragma endregion RenderCommandEncoder