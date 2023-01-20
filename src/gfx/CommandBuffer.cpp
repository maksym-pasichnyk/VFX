#include "Device.hpp"
#include "Swapchain.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

gfx::CommandBufferShared::CommandBufferShared(gfx::Device device, gfx::CommandQueue queue) : device(device), queue(queue) {
    vk::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.setCommandPool(queue.shared->raw);
    allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
    allocate_info.setCommandBufferCount(1);
    vk::resultCheck(device.handle().allocateCommandBuffers(&allocate_info, &raw, device.dispatcher()), "allocateCommandBuffers");

    vk::FenceCreateInfo fence_create_info = {};
    fence = device.handle().createFence(fence_create_info, nullptr, device.dispatcher());

    vk::SemaphoreCreateInfo semaphore_create_info = {};
    semaphore = device.handle().createSemaphore(semaphore_create_info, nullptr, device.dispatcher());
}

gfx::CommandBufferShared::~CommandBufferShared() {
    device.handle().destroySemaphore(semaphore, nullptr, device.dispatcher());
    device.handle().destroyFence(fence, nullptr, device.dispatcher());
}

void gfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    shared->raw.begin(info, shared->device.dispatcher());
}

void gfx::CommandBuffer::end() {
    shared->raw.end(shared->device.dispatcher());
}

void gfx::CommandBuffer::submit() {
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&shared->raw);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&shared->semaphore);

    vk::resultCheck(shared->device.handle().resetFences(1, &shared->fence, shared->device.dispatcher()), "Submit");

    // todo: get valid device for submit
    shared->device.shared->raw_queue.submit(submit_info, shared->fence, shared->device.dispatcher());
}

void gfx::CommandBuffer::present(const gfx::Drawable& drawable) {
    vk::PresentInfoKHR present_info = {};
    present_info.setWaitSemaphores(shared->semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable.swapchain);
    present_info.setPImageIndices(&drawable.drawableIndex);

    // todo: get valid device for present
    vk::Result result = shared->device.shared->raw_queue.presentKHR(present_info, shared->device.dispatcher());
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void gfx::CommandBuffer::setComputePipelineState(ComputePipelineState const& state) {
    shared->raw.bindPipeline(vk::PipelineBindPoint::eCompute, state.shared->pipeline, shared->device.dispatcher());

    shared->pipeline = state.shared->pipeline;
    shared->pipeline_layout = state.shared->pipeline_layout;
    shared->pipeline_bind_point = vk::PipelineBindPoint::eCompute;
}

void gfx::CommandBuffer::bindDescriptorSet(const DescriptorSet& descriptorSet, uint32_t slot) {
    shared->raw.bindDescriptorSets(shared->pipeline_bind_point, shared->pipeline_layout, slot, 1, &descriptorSet.shared->raw, 0, nullptr, shared->device.dispatcher());
}

void gfx::CommandBuffer::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    shared->raw.pushConstants(shared->pipeline_layout, stageFlags, offset, size, data, shared->device.dispatcher());
}

void gfx::CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    shared->raw.dispatch(groupCountX, groupCountY, groupCountZ, shared->device.dispatcher());
}

void gfx::CommandBuffer::waitUntilCompleted() {
    vk::Result result = shared->device.handle().waitForFences(shared->fence, VK_TRUE, std::numeric_limits<uint64_t>::max(), shared->device.dispatcher());
    vk::resultCheck(result, "waitForFences");
}

void gfx::CommandBuffer::imageBarrier(Texture const& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask) {
    vk::ImageMemoryBarrier2 barrier = {};
    barrier.setSrcStageMask(srcStageMask);
    barrier.setSrcAccessMask(srcAccessMask);
    barrier.setDstStageMask(dstStageMask);
    barrier.setDstAccessMask(dstAccessMask);
    barrier.setOldLayout(oldLayout);
    barrier.setNewLayout(newLayout);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setImage(texture.shared->image);
    barrier.setSubresourceRange(texture.shared->subresource);

    vk::DependencyInfo dependency_info = {};
    dependency_info.setImageMemoryBarrierCount(1);
    dependency_info.setPImageMemoryBarriers(&barrier);

    shared->raw.pipelineBarrier2(dependency_info, shared->device.dispatcher());
}

void gfx::CommandBuffer::beginRendering(const RenderingInfo& info) {
    vk::RenderingAttachmentInfo depthAttachment = {};
    vk::RenderingAttachmentInfo stencilAttachment = {};
    std::vector<vk::RenderingAttachmentInfo> colorAttachments = {};

    colorAttachments.resize(info.colorAttachments.elements.size());
    for (uint64_t i = 0; i < info.colorAttachments.elements.size(); ++i) {
        auto rgba = std::array{
            info.colorAttachments.elements[i].clearColor.red,
            info.colorAttachments.elements[i].clearColor.greed,
            info.colorAttachments.elements[i].clearColor.blue,
            info.colorAttachments.elements[i].clearColor.alpha
        };

        vk::ClearColorValue color = {};
        color.setFloat32(rgba);

        if (info.colorAttachments.elements[i].texture) {
            colorAttachments[i].setImageView(info.colorAttachments.elements[i].texture->shared->image_view);
            colorAttachments[i].setImageLayout(info.colorAttachments.elements[i].imageLayout);
        }
        if (info.colorAttachments.elements[i].resolveTexture) {
            colorAttachments[i].setResolveMode(info.colorAttachments.elements[i].resolveMode);
            colorAttachments[i].setResolveImageView(info.colorAttachments.elements[i].resolveTexture->shared->image_view);
            colorAttachments[i].setResolveImageLayout(info.colorAttachments.elements[i].resolveImageLayout);
        }
        colorAttachments[i].setLoadOp(info.colorAttachments.elements[i].loadOp);
        colorAttachments[i].setStoreOp(info.colorAttachments.elements[i].storeOp);
        colorAttachments[i].clearValue.setColor(color);
    }

    vk::RenderingInfo rendering_info = {};
    rendering_info.setRenderArea(info.renderArea);
    rendering_info.setLayerCount(info.layerCount);
    rendering_info.setViewMask(info.viewMask);
    rendering_info.setColorAttachments(colorAttachments);

    if (info.depthAttachment.texture || info.depthAttachment.resolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setDepth(info.depthAttachment.clearDepth);

        if (info.depthAttachment.texture) {
            depthAttachment.setImageView(info.depthAttachment.texture->shared->image_view);
            depthAttachment.setImageLayout(info.depthAttachment.imageLayout);
        }
        if (info.depthAttachment.resolveTexture) {
            depthAttachment.setResolveMode(info.depthAttachment.resolveMode);
            depthAttachment.setResolveImageView(info.depthAttachment.resolveTexture->shared->image_view);
            depthAttachment.setResolveImageLayout(info.depthAttachment.resolveImageLayout);
        }
        depthAttachment.setLoadOp(info.depthAttachment.loadOp);
        depthAttachment.setStoreOp(info.depthAttachment.storeOp);
        depthAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPDepthAttachment(&depthAttachment);
    }

    if (info.stencilAttachment.texture || info.stencilAttachment.resolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setDepth(info.stencilAttachment.clearStencil);

        if (info.stencilAttachment.texture) {
            stencilAttachment.setImageView(info.stencilAttachment.texture->shared->image_view);
            stencilAttachment.setImageLayout(info.stencilAttachment.imageLayout);
        }
        if (info.stencilAttachment.resolveTexture) {
            stencilAttachment.setResolveMode(info.stencilAttachment.resolveMode);
            stencilAttachment.setResolveImageView(info.stencilAttachment.resolveTexture->shared->image_view);
            stencilAttachment.setResolveImageLayout(info.stencilAttachment.resolveImageLayout);
        }
        stencilAttachment.setLoadOp(info.stencilAttachment.loadOp);
        stencilAttachment.setStoreOp(info.stencilAttachment.storeOp);
        stencilAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPStencilAttachment(&stencilAttachment);
    }

    shared->raw.beginRendering(rendering_info, shared->device.dispatcher());

}

void gfx::CommandBuffer::endRendering() {
    shared->raw.endRendering(shared->device.dispatcher());
}

void gfx::CommandBuffer::setRenderPipelineState(const RenderPipelineState& state) {
    shared->raw.bindPipeline(vk::PipelineBindPoint::eGraphics, state.shared->pipeline, shared->device.dispatcher());

    shared->pipeline = state.shared->pipeline;
    shared->pipeline_layout = state.shared->pipeline_layout;
    shared->pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
}

void gfx::CommandBuffer::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    shared->raw.setScissor(firstScissor, 1, &rect, shared->device.dispatcher());
}

void gfx::CommandBuffer::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    shared->raw.setViewport(firstViewport, 1, &viewport, shared->device.dispatcher());
}

void gfx::CommandBuffer::bindIndexBuffer(const Buffer& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    shared->raw.bindIndexBuffer(buffer.shared->raw, offset, indexType, shared->device.dispatcher());
}

void gfx::CommandBuffer::bindVertexBuffer(int firstBinding, const Buffer& buffer, vk::DeviceSize offset) {
    shared->raw.bindVertexBuffers(firstBinding, 1, &buffer.shared->raw, &offset, shared->device.dispatcher());
}

void gfx::CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    shared->raw.draw(vertexCount, instanceCount, firstVertex, firstInstance, shared->device.dispatcher());
}

void gfx::CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    shared->raw.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, shared->device.dispatcher());
}
