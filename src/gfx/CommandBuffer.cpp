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

gfx::CommandBufferShared::CommandBufferShared(Device device, CommandQueue queue) : device(device), queue(queue) {
    vk::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.setCommandPool(queue.shared->raw);
    allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
    allocate_info.setCommandBufferCount(1);
    vk::resultCheck(device.shared->raii.raw.allocateCommandBuffers(&allocate_info, &raw, device.shared->raii.dispatcher), "allocateCommandBuffers");

    fence       = device.shared->raii.raw.createFence(vk::FenceCreateInfo(), nullptr, device.shared->raii.dispatcher);
    semaphore   = device.shared->raii.raw.createSemaphore(vk::SemaphoreCreateInfo(), nullptr, device.shared->raii.dispatcher);


//    vk::DescriptorPoolCreateInfo pool_create_info = {};
//    pool_create_info.setMaxSets(1024);
//    pool_create_info.setPoolSizes(pool_sizes);
//    auto pool = shared->raii.raw.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, shared->raii.dispatcher);
}

gfx::CommandBufferShared::~CommandBufferShared() {
    device.shared->raii.raw.destroySemaphore(semaphore, nullptr, device.shared->raii.dispatcher);
    device.shared->raii.raw.destroyFence(fence, nullptr, device.shared->raii.dispatcher);

    for (auto& value : descriptor_pools) {
        device.shared->raii.raw.destroyDescriptorPool(value, nullptr, device.shared->raii.dispatcher);
    }
}

void gfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    shared->raw.begin(info, shared->device.shared->raii.dispatcher);

    for (auto& value : shared->descriptor_pools) {
        shared->device.shared->raii.raw.resetDescriptorPool(value, {}, shared->device.shared->raii.dispatcher);
    }
}

void gfx::CommandBuffer::end() {
    shared->raw.end(shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::submit() {
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&shared->raw);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&shared->semaphore);

    vk::resultCheck(shared->device.shared->raii.raw.resetFences(1, &shared->fence, shared->device.shared->raii.dispatcher), "Submit");

    // todo: get valid device for submit
    shared->device.shared->queue.submit(submit_info, shared->fence, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::present(const gfx::Drawable& drawable) {
    vk::PresentInfoKHR present_info = {};
    present_info.setWaitSemaphores(shared->semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable.swapchain);
    present_info.setPImageIndices(&drawable.drawableIndex);

    // todo: get valid device for present
    vk::Result result = shared->device.shared->queue.presentKHR(present_info, shared->device.shared->raii.dispatcher);
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void gfx::CommandBuffer::setComputePipelineState(ComputePipelineState const& state) {
    shared->raw.bindPipeline(vk::PipelineBindPoint::eCompute, state.shared->pipeline, shared->device.shared->raii.dispatcher);

    shared->pipeline = state.shared->pipeline;
    shared->pipeline_layout = state.shared->pipeline_layout;
    shared->pipeline_bind_point = vk::PipelineBindPoint::eCompute;
}

void gfx::CommandBuffer::bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot) {
    shared->raw.bindDescriptorSets(shared->pipeline_bind_point, shared->pipeline_layout, slot, 1, &descriptorSet, 0, nullptr, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    shared->raw.pushConstants(shared->pipeline_layout, stageFlags, offset, size, data, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    shared->raw.dispatch(groupCountX, groupCountY, groupCountZ, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::waitUntilCompleted() {
    vk::Result result = shared->device.shared->raii.raw.waitForFences(shared->fence, VK_TRUE, std::numeric_limits<uint64_t>::max(), shared->device.shared->raii.dispatcher);
    vk::resultCheck(result, "waitForFences");
}

void gfx::CommandBuffer::setImageLayout(Texture const& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask) {
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

    shared->raw.pipelineBarrier2(dependency_info, shared->device.shared->raii.dispatcher);
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

    shared->raw.beginRendering(rendering_info, shared->device.shared->raii.dispatcher);

}

void gfx::CommandBuffer::endRendering() {
    shared->raw.endRendering(shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::setRenderPipelineState(const RenderPipelineState& state) {
    shared->raw.bindPipeline(vk::PipelineBindPoint::eGraphics, state.shared->pipeline, shared->device.shared->raii.dispatcher);

    shared->pipeline = state.shared->pipeline;
    shared->pipeline_layout = state.shared->pipeline_layout;
    shared->pipeline_bind_point = vk::PipelineBindPoint::eGraphics;
}

void gfx::CommandBuffer::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    shared->raw.setScissor(firstScissor, 1, &rect, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    shared->raw.setViewport(firstViewport, 1, &viewport, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::bindIndexBuffer(const Buffer& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    shared->raw.bindIndexBuffer(buffer.shared->raw, offset, indexType, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::bindVertexBuffer(int firstBinding, const Buffer& buffer, vk::DeviceSize offset) {
    shared->raw.bindVertexBuffers(firstBinding, 1, &buffer.shared->raw, &offset, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    shared->raw.draw(vertexCount, instanceCount, firstVertex, firstInstance, shared->device.shared->raii.dispatcher);
}

void gfx::CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    shared->raw.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, shared->device.shared->raii.dispatcher);
}

// todo: get sizes from layout
auto gfx::CommandBuffer::newDescriptorSet(vk::DescriptorSetLayout layout, const std::vector<vk::DescriptorPoolSize>& sizes) -> vk::DescriptorSet {
    for (auto pool : shared->descriptor_pools) {
        vk::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.setDescriptorPool(pool);
        allocate_info.setDescriptorSetCount(1);
        allocate_info.setPSetLayouts(&layout);

        vk::DescriptorSet descriptor_set;
        vk::Result result = shared->device.shared->raii.raw.allocateDescriptorSets(&allocate_info, &descriptor_set, shared->device.shared->raii.dispatcher);
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
    pool_create_info.setPoolSizes(sizes);

    auto pool = shared->device.shared->raii.raw.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, shared->device.shared->raii.dispatcher);
    shared->descriptor_pools.push_back(pool);

    vk::DescriptorSetAllocateInfo allocate_info = {};
    allocate_info.setDescriptorPool(pool);
    allocate_info.setDescriptorSetCount(1);
    allocate_info.setPSetLayouts(&layout);

    vk::DescriptorSet descriptor_set;
    vk::Result result = shared->device.shared->raii.raw.allocateDescriptorSets(&allocate_info, &descriptor_set, shared->device.shared->raii.dispatcher);
    if (result == vk::Result::eSuccess) {
        return descriptor_set;
    }
    return VK_NULL_HANDLE;
}