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
    allocate_info.setCommandPool(pCommandQueue->vkCommandPool);
    allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
    allocate_info.setCommandBufferCount(1);

    vk::resultCheck(mDevice->vkDevice.allocateCommandBuffers(&allocate_info, &vkCommandBuffer, mDevice->vkDispatchLoaderDynamic), "allocateCommandBuffers");

    vk::FenceCreateInfo fence_create_info = {};
    vkFence = mDevice->vkDevice.createFence(fence_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);

    vk::SemaphoreCreateInfo semaphore_create_info = {};
    vkSemaphore = mDevice->vkDevice.createSemaphore(semaphore_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);
}

gfx::CommandBuffer::~CommandBuffer() {
    mDevice->vkDevice.destroySemaphore(vkSemaphore, nullptr, mDevice->vkDispatchLoaderDynamic);
    mDevice->vkDevice.destroyFence(vkFence, nullptr, mDevice->vkDispatchLoaderDynamic);
}

auto gfx::CommandBuffer::getRetainedReferences() -> bool {
    return mRetainedReferences;
}

void gfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    vkCommandBuffer.begin(info, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::end() {
    vkCommandBuffer.end(mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::submit() {
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&vkCommandBuffer);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&vkSemaphore);

    vk::resultCheck(mDevice->vkDevice.resetFences(1, &vkFence, mDevice->vkDispatchLoaderDynamic), "Submit");
    pCommandQueue->vkGraphicsQueue.submit(submit_info, vkFence, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::present(const SharedPtr<gfx::Drawable>& drawable) {
    vk::PresentInfoKHR present_info = {};
    present_info.setWaitSemaphores(vkSemaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable->pLayer->vkSwapchain);
    present_info.setPImageIndices(&drawable->mDrawableIndex);

    vk::Result result = pCommandQueue->vkPresentQueue.presentKHR(present_info, mDevice->vkDispatchLoaderDynamic);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        mDevice->waitIdle();
        drawable->pLayer->releaseDrawables();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void gfx::CommandBuffer::setComputePipelineState(const SharedPtr<ComputePipelineState>& state) {
    vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, state->vkPipeline, mDevice->vkDispatchLoaderDynamic);

    vkPipeline = state->vkPipeline;
    vkPipelineLayout = state->vkPipelineLayout;
    vkPipelineBindPoint = vk::PipelineBindPoint::eCompute;
}

void gfx::CommandBuffer::bindDescriptorSet(const SharedPtr<DescriptorSet>& descriptorSet, uint32_t slot) {
    vkCommandBuffer.bindDescriptorSets(vkPipelineBindPoint, vkPipelineLayout, slot, 1, &descriptorSet->vkDescriptorSet, 0, nullptr, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    vkCommandBuffer.pushConstants(vkPipelineLayout, stageFlags, offset, size, data, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    vkCommandBuffer.dispatch(groupCountX, groupCountY, groupCountZ, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::waitUntilCompleted() {
    vk::Result result = pCommandQueue->mDevice->vkDevice.waitForFences(vkFence, VK_TRUE, std::numeric_limits<uint64_t>::max(), mDevice->vkDispatchLoaderDynamic);
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
    barrier.setImage(texture->vkImage);
    barrier.setSubresourceRange(texture->vkImageSubresourceRange);

    vk::DependencyInfo dependency_info = {};
    dependency_info.setImageMemoryBarrierCount(1);
    dependency_info.setPImageMemoryBarriers(&barrier);

    vkCommandBuffer.pipelineBarrier2(dependency_info, mDevice->vkDispatchLoaderDynamic);
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
            colorAttachments[i].setImageView(info.mColorAttachments.elements[i].mTexture->vkImageView);
            colorAttachments[i].setImageLayout(info.mColorAttachments.elements[i].vkImageLayout);
        }
        if (info.mColorAttachments.elements[i].mResolveTexture) {
            colorAttachments[i].setResolveMode(info.mColorAttachments.elements[i].vkResolveMode);
            colorAttachments[i].setResolveImageView(info.mColorAttachments.elements[i].mResolveTexture->vkImageView);
            colorAttachments[i].setResolveImageLayout(info.mColorAttachments.elements[i].vkResolveImageLayout);
        }
        colorAttachments[i].setLoadOp(info.mColorAttachments.elements[i].vkLoadOp);
        colorAttachments[i].setStoreOp(info.mColorAttachments.elements[i].vkStoreOp);
        colorAttachments[i].clearValue.setColor(color);
    }

    vk::RenderingInfo rendering_info = {};
    rendering_info.setRenderArea(info.vkRenderArea);
    rendering_info.setLayerCount(info.mLayerCount);
    rendering_info.setViewMask(info.mViewMask);
    rendering_info.setColorAttachments(colorAttachments);

    if (info.mDepthAttachment.mTexture || info.mDepthAttachment.mResolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setDepth(info.mDepthAttachment.mClearDepth);

        if (info.mDepthAttachment.mTexture) {
            depthAttachment.setImageView(info.mDepthAttachment.mTexture->vkImageView);
            depthAttachment.setImageLayout(info.mDepthAttachment.vkImageLayout);
        }
        if (info.mDepthAttachment.mResolveTexture) {
            depthAttachment.setResolveMode(info.mDepthAttachment.vkResolveMode);
            depthAttachment.setResolveImageView(info.mDepthAttachment.mResolveTexture->vkImageView);
            depthAttachment.setResolveImageLayout(info.mDepthAttachment.vkResolveImageLayout);
        }
        depthAttachment.setLoadOp(info.mDepthAttachment.vkLoadOp);
        depthAttachment.setStoreOp(info.mDepthAttachment.vkStoreOp);
        depthAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPDepthAttachment(&depthAttachment);
    }

    if (info.mStencilAttachment.mTexture || info.mStencilAttachment.mResolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setDepth(info.mStencilAttachment.mClearStencil);

        if (info.mStencilAttachment.mTexture) {
            stencilAttachment.setImageView(info.mStencilAttachment.mTexture->vkImageView);
            stencilAttachment.setImageLayout(info.mStencilAttachment.vkImageLayout);
        }
        if (info.mStencilAttachment.mResolveTexture) {
            stencilAttachment.setResolveMode(info.mStencilAttachment.vkResolveMode);
            stencilAttachment.setResolveImageView(info.mStencilAttachment.mResolveTexture->vkImageView);
            stencilAttachment.setResolveImageLayout(info.mStencilAttachment.vkResolveImageLayout);
        }
        stencilAttachment.setLoadOp(info.mStencilAttachment.vkLoadOp);
        stencilAttachment.setStoreOp(info.mStencilAttachment.vkStoreOp);
        stencilAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPStencilAttachment(&stencilAttachment);
    }

    vkCommandBuffer.beginRendering(rendering_info, mDevice->vkDispatchLoaderDynamic);

}

void gfx::CommandBuffer::endRendering() {
    vkCommandBuffer.endRendering(mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::setRenderPipelineState(const SharedPtr<RenderPipelineState>& state) {
    vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, state->vkPipeline, mDevice->vkDispatchLoaderDynamic);

    vkPipeline = state->vkPipeline;
    vkPipelineLayout = state->vkPipelineLayout;
    vkPipelineBindPoint = vk::PipelineBindPoint::eGraphics;
}

void gfx::CommandBuffer::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    vkCommandBuffer.setScissor(firstScissor, 1, &rect, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    vkCommandBuffer.setViewport(firstViewport, 1, &viewport, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::bindIndexBuffer(const SharedPtr<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    vkCommandBuffer.bindIndexBuffer(buffer->vkBuffer, offset, indexType, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::bindVertexBuffer(int firstBinding, const SharedPtr<Buffer>& buffer, vk::DeviceSize offset) {
    vkCommandBuffer.bindVertexBuffers(firstBinding, 1, &buffer->vkBuffer, &offset, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vkCommandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance, mDevice->vkDispatchLoaderDynamic);
}

void gfx::CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    vkCommandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, mDevice->vkDispatchLoaderDynamic);
}
#pragma endregion RenderCommandEncoder