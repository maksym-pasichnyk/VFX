#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"
#include "Swapchain.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "DescriptorSet.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

gfx::CommandBuffer::CommandBuffer(ManagedShared<Device> device, ManagedShared<CommandQueue> queue) : device(device), queue(queue) {
    vk::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.setCommandPool(queue->raw);
    allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
    allocate_info.setCommandBufferCount(1);
    vk::resultCheck(device->raii.raw.allocateCommandBuffers(&allocate_info, &raw, device->raii.dispatcher), "Failed to allocate command buffer");

    vk::FenceCreateInfo fence_create_info = {};
    vk::resultCheck(device->raii.raw.createFence(&fence_create_info, nullptr, &fence, device->raii.dispatcher), "Failed to create fence");

    vk::SemaphoreCreateInfo semaphore_create_info = {};
    vk::resultCheck(device->raii.raw.createSemaphore(&semaphore_create_info, nullptr, &semaphore, device->raii.dispatcher), "Failed to create semaphore");

//    vk::DescriptorPoolCreateInfo pool_create_info = {};
//    pool_create_info.setMaxSets(1024);
//    pool_create_info.setPoolSizes(pool_sizes);
//    auto pool = raii.raw.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, raii.dispatcher);
}

gfx::CommandBuffer::~CommandBuffer() {
    device->raii.raw.destroySemaphore(semaphore, nullptr, device->raii.dispatcher);
    device->raii.raw.destroyFence(fence, nullptr, device->raii.dispatcher);

    for (auto& value : descriptor_pools) {
        device->raii.raw.destroyDescriptorPool(value, nullptr, device->raii.dispatcher);
    }
}

void gfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    raw.begin(info, device->raii.dispatcher);

    for (auto& value : descriptor_pools) {
        device->raii.raw.resetDescriptorPool(value, {}, device->raii.dispatcher);
    }
}

void gfx::CommandBuffer::end() {
    raw.end(device->raii.dispatcher);
}

void gfx::CommandBuffer::submit() {
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&raw);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&semaphore);

    vk::resultCheck(device->raii.raw.resetFences(1, &fence, device->raii.dispatcher), "Failed to reset fence");

    // todo: get valid device for submit
    device->queue.submit(submit_info, fence, device->raii.dispatcher);
}

void gfx::CommandBuffer::present(const gfx::Drawable& drawable) {
    vk::PresentInfoKHR present_info = {};
    present_info.setWaitSemaphores(semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable.swapchain);
    present_info.setPImageIndices(&drawable.drawableIndex);

    // todo: get valid device for present
    vk::Result result = device->queue.presentKHR(present_info, device->raii.dispatcher);
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void gfx::CommandBuffer::setComputePipelineState(const ManagedShared<ComputePipelineState>& state) {
    raw.bindPipeline(vk::PipelineBindPoint::eCompute, state->pipeline, device->raii.dispatcher);

    pipeline = state->pipeline;
    pipeline_layout = state->pipeline_layout;
    pipeline_bind_point = vk::PipelineBindPoint::eCompute;
}

void gfx::CommandBuffer::bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot) {
    raw.bindDescriptorSets(pipeline_bind_point, pipeline_layout, slot, 1, &descriptorSet, 0, nullptr, device->raii.dispatcher);
}

void gfx::CommandBuffer::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    raw.pushConstants(pipeline_layout, stageFlags, offset, size, data, device->raii.dispatcher);
}

void gfx::CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    raw.dispatch(groupCountX, groupCountY, groupCountZ, device->raii.dispatcher);
}

void gfx::CommandBuffer::waitUntilCompleted() {
    vk::resultCheck(device->raii.raw.waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max(), device->raii.dispatcher), "Failed to wait for fence");
}

void gfx::CommandBuffer::setImageLayout(const ManagedShared<Texture>& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask) {
    vk::ImageMemoryBarrier2 barrier = {};
    barrier.setSrcStageMask(srcStageMask);
    barrier.setSrcAccessMask(srcAccessMask);
    barrier.setDstStageMask(dstStageMask);
    barrier.setDstAccessMask(dstAccessMask);
    barrier.setOldLayout(oldLayout);
    barrier.setNewLayout(newLayout);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setImage(texture->image);
    barrier.setSubresourceRange(texture->subresource);

    vk::DependencyInfo dependency_info = {};
    dependency_info.setImageMemoryBarrierCount(1);
    dependency_info.setPImageMemoryBarriers(&barrier);

    raw.pipelineBarrier2(dependency_info, device->raii.dispatcher);
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
            colorAttachments[i].setImageView(info.colorAttachments.elements[i].texture->image_view);
            colorAttachments[i].setImageLayout(info.colorAttachments.elements[i].imageLayout);
        }
        if (info.colorAttachments.elements[i].resolveTexture) {
            colorAttachments[i].setResolveMode(info.colorAttachments.elements[i].resolveMode);
            colorAttachments[i].setResolveImageView(info.colorAttachments.elements[i].resolveTexture->image_view);
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
            depthAttachment.setImageView(info.depthAttachment.texture->image_view);
            depthAttachment.setImageLayout(info.depthAttachment.imageLayout);
        }
        if (info.depthAttachment.resolveTexture) {
            depthAttachment.setResolveMode(info.depthAttachment.resolveMode);
            depthAttachment.setResolveImageView(info.depthAttachment.resolveTexture->image_view);
            depthAttachment.setResolveImageLayout(info.depthAttachment.resolveImageLayout);
        }
        depthAttachment.setLoadOp(info.depthAttachment.loadOp);
        depthAttachment.setStoreOp(info.depthAttachment.storeOp);
        depthAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPDepthAttachment(&depthAttachment);
    }

    if (info.stencilAttachment.texture || info.stencilAttachment.resolveTexture) {
        vk::ClearDepthStencilValue depth_stencil = {};
        depth_stencil.setStencil(info.stencilAttachment.clearStencil);

        if (info.stencilAttachment.texture) {
            stencilAttachment.setImageView(info.stencilAttachment.texture->image_view);
            stencilAttachment.setImageLayout(info.stencilAttachment.imageLayout);
        }
        if (info.stencilAttachment.resolveTexture) {
            stencilAttachment.setResolveMode(info.stencilAttachment.resolveMode);
            stencilAttachment.setResolveImageView(info.stencilAttachment.resolveTexture->image_view);
            stencilAttachment.setResolveImageLayout(info.stencilAttachment.resolveImageLayout);
        }
        stencilAttachment.setLoadOp(info.stencilAttachment.loadOp);
        stencilAttachment.setStoreOp(info.stencilAttachment.storeOp);
        stencilAttachment.clearValue.setDepthStencil(depth_stencil);

        rendering_info.setPStencilAttachment(&stencilAttachment);
    }

    raw.beginRendering(rendering_info, device->raii.dispatcher);
}

void gfx::CommandBuffer::endRendering() {
    raw.endRendering(device->raii.dispatcher);
}

void gfx::CommandBuffer::setRenderPipelineState(const ManagedShared<RenderPipelineState>& state) {
    raw.bindPipeline(vk::PipelineBindPoint::eGraphics, state->pipeline, device->raii.dispatcher);

    pipeline = state->pipeline;
    pipeline_layout = state->pipeline_layout;
    pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
}

void gfx::CommandBuffer::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    raw.setScissor(firstScissor, 1, &rect, device->raii.dispatcher);
}

void gfx::CommandBuffer::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    raw.setViewport(firstViewport, 1, &viewport, device->raii.dispatcher);
}

void gfx::CommandBuffer::bindIndexBuffer(const ManagedShared<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    raw.bindIndexBuffer(buffer->raw, offset, indexType, device->raii.dispatcher);
}

void gfx::CommandBuffer::bindVertexBuffer(int firstBinding, const ManagedShared<Buffer>& buffer, vk::DeviceSize offset) {
    raw.bindVertexBuffers(firstBinding, 1, &buffer->raw, &offset, device->raii.dispatcher);
}

void gfx::CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    raw.draw(vertexCount, instanceCount, firstVertex, firstInstance, device->raii.dispatcher);
}

void gfx::CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    raw.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, device->raii.dispatcher);
}

// todo: get sizes from layout
auto gfx::CommandBuffer::newDescriptorSet(const ManagedShared<RenderPipelineState>& render_pipeline_state, uint32_t index) -> vk::DescriptorSet {
    for (auto pool : descriptor_pools) {
        vk::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.setDescriptorPool(pool);
        allocate_info.setDescriptorSetCount(1);
        allocate_info.setPSetLayouts(&render_pipeline_state->bind_group_layouts[index]);

        vk::DescriptorSet descriptor_set = VK_NULL_HANDLE;
        vk::Result result = device->raii.raw.allocateDescriptorSets(&allocate_info, &descriptor_set, device->raii.dispatcher);
        if (result == vk::Result::eSuccess) {
            return descriptor_set;
        }
    }

    // todo: reduce pool sizes, check is there is enough space for the new descriptor set
    auto pool_sizes = std::array{
        vk::DescriptorPoolSize{vk::DescriptorType::eSampler                  , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage             , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler     , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage             , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer       , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer       , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer            , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer            , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic     , 1024},
        vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic     , 1024}
    };

    vk::DescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.setMaxSets(1024);
    pool_create_info.setPoolSizes(pool_sizes);

    auto pool = device->raii.raw.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, device->raii.dispatcher);
    descriptor_pools.push_back(pool);

    vk::DescriptorSetAllocateInfo allocate_info = {};
    allocate_info.setDescriptorPool(pool);
    allocate_info.setDescriptorSetCount(1);
    allocate_info.setPSetLayouts(&render_pipeline_state->bind_group_layouts[index]);

    vk::DescriptorSet descriptor_set = VK_NULL_HANDLE;
    vk::Result result = device->raii.raw.allocateDescriptorSets(&allocate_info, &descriptor_set, device->raii.dispatcher);
    if (result == vk::Result::eSuccess) {
        return descriptor_set;
    }
    return VK_NULL_HANDLE;
}