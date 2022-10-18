#include "pass.hpp"
#include "queue.hpp"
#include "window.hpp"
#include "buffer.hpp"
#include "context.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "drawable.hpp"
#include "swapchain.hpp"

#include "spdlog/spdlog.h"

namespace {
    void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const vfx::RenderingColorAttachmentInfo& in) {
        auto clearValue = vk::ClearColorValue{
            .float32 = std::array{
                in.clearColor.red,
                in.clearColor.greed,
                in.clearColor.blue,
                in.clearColor.alpha
            }
        };

        out.imageView = in.texture ? in.texture->view : VK_NULL_HANDLE;
        out.imageLayout = in.imageLayout;
        out.resolveMode = in.resolveMode;
        out.resolveImageView = in.resolveTexture ? in.resolveTexture->view : VK_NULL_HANDLE;
        out.resolveImageLayout = in.resolveImageLayout;
        out.loadOp = in.loadOp;
        out.storeOp = in.storeOp;
        out.clearValue.setColor(clearValue);
    }

    void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const vfx::RenderingDepthAttachmentInfo& in) {
        auto clear_value = vk::ClearDepthStencilValue {
            .depth = in.clearDepth
        };

        out.imageView = in.texture ? in.texture->view : VK_NULL_HANDLE;
        out.imageLayout = in.imageLayout;
        out.resolveMode = in.resolveMode;
        out.resolveImageView = in.resolveTexture ? in.resolveTexture->view : VK_NULL_HANDLE;
        out.resolveImageLayout = in.resolveImageLayout;
        out.loadOp = in.loadOp;
        out.storeOp = in.storeOp;
        out.clearValue.setDepthStencil(clear_value);
    }

    void fillAttachmentInfo(vk::RenderingAttachmentInfo& out, const vfx::RenderingStencilAttachmentInfo& in) {
        auto clearValue = vk::ClearDepthStencilValue {
            .stencil = in.clearStencil
        };

        out.imageView = in.texture ? in.texture->view : VK_NULL_HANDLE;
        out.imageLayout = in.imageLayout;
        out.resolveMode = in.resolveMode;
        out.resolveImageView = in.resolveTexture ? in.resolveTexture->view : VK_NULL_HANDLE;
        out.resolveImageLayout = in.resolveImageLayout;
        out.loadOp = in.loadOp;
        out.storeOp = in.storeOp;
        out.clearValue.setDepthStencil(clearValue);
    }
}

void vfx::CommandBuffer::reset() {
}

void vfx::CommandBuffer::releaseReferences() {
    bufferReferences.clear();
    textureReferences.clear();
}

//auto vfx::CommandBuffer::makePipeline(i32 subpass) -> vk::Pipeline {
//    auto key = std::make_tuple(pipelineState, renderPass, subpass);
//    if (auto it = commandQueue->pipelines.find(key); it != commandQueue->pipelines.end()) {
//        return it->second;
//    }
//
//    vk::PipelineViewportStateCreateInfo viewportState = {};
//    viewportState.setViewportCount(1);
//    viewportState.setScissorCount(1);
//
//    std::array dynamicStates = {
//        vk::DynamicState::eViewport,
//        vk::DynamicState::eScissor
//    };
//
//    vk::PipelineDynamicStateCreateInfo dynamicState = {};
//    dynamicState.setDynamicStates(dynamicStates);
//
//    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};
//
//    if (pipelineState->description.vertexFunction) {
//        vk::PipelineShaderStageCreateInfo info{};
//        info.setStage(vk::ShaderStageFlagBits::eVertex);
//        info.setModule(pipelineState->description.vertexFunction->library->module);
//        info.setPName(pipelineState->description.vertexFunction->name.c_str());
//        stages.emplace_back(info);
//    }
//
//    if (pipelineState->description.fragmentFunction) {
//        vk::PipelineShaderStageCreateInfo info{};
//        info.setStage(vk::ShaderStageFlagBits::eFragment);
//        info.setModule(pipelineState->description.fragmentFunction->library->module);
//        info.setPName(pipelineState->description.fragmentFunction->name.c_str());
//        stages.emplace_back(info);
//    }
//
//    vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
//    vertexInputState.setVertexBindingDescriptions(pipelineState->description.bindings);
//    vertexInputState.setVertexAttributeDescriptions(pipelineState->description.attributes);
//
//    vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
//    colorBlendState.setAttachments(pipelineState->description.attachments.elements);
//
//    vk::PipelineRenderingCreateInfo rendering = {};
//    rendering.setViewMask(pipelineState->description.viewMask);
//    rendering.setColorAttachmentFormats(pipelineState->description.colorAttachmentFormats.elements);
//    rendering.setDepthAttachmentFormat(pipelineState->description.depthAttachmentFormat);
//    rendering.setStencilAttachmentFormat(pipelineState->description.stencilAttachmentFormat);
//
//    auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{};
//    pipeline_create_info.setPNext(&rendering);
//    pipeline_create_info.setStages(stages);
//    pipeline_create_info.setPVertexInputState(&vertexInputState);
//    pipeline_create_info.setPInputAssemblyState(&pipelineState->description.inputAssemblyState);
//    pipeline_create_info.setPViewportState(&viewportState);
//    pipeline_create_info.setPRasterizationState(&pipelineState->description.rasterizationState);
//    pipeline_create_info.setPMultisampleState(&pipelineState->description.multisampleState);
//    pipeline_create_info.setPDepthStencilState(&pipelineState->description.depthStencilState);
//    pipeline_create_info.setPColorBlendState(&colorBlendState);
//    pipeline_create_info.setPDynamicState(&dynamicState);
//    pipeline_create_info.setLayout(pipelineState->pipelineLayout);
//    pipeline_create_info.setRenderPass(renderPass);
//    pipeline_create_info.setSubpass(subpass);
//    pipeline_create_info.setBasePipelineHandle(nullptr);
//    pipeline_create_info.setBasePipelineIndex(0);
//
//    vk::Pipeline pipeline{};
//    auto result = commandQueue->context->logical_device.createGraphicsPipelines(
//        {},
//        1,
//        &pipeline_create_info,
//        nullptr,
//        &pipeline
//    );
//    if (result != vk::Result::eSuccess) {
//        throw std::runtime_error(vk::to_string(result));
//    }
//    spdlog::info("Created pipeline (state = {}, pass = {}, subpass = {})", (void*)pipelineState.get(), (void*)renderPass, subpass);
//    commandQueue->pipelines.emplace(key, pipeline);
//    return pipeline;
//}

auto vfx::CommandBuffer::getRetainedReferences() const -> bool {
    return retainedReferences;
}

void vfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    handle.begin(info);
}

void vfx::CommandBuffer::end() {
    handle.end();
}

void vfx::CommandBuffer::submit() {
    auto submit_info = vk::SubmitInfo{};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&handle);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&semaphore);

    commandQueue->handle.submit(submit_info, fence);
}

void vfx::CommandBuffer::present(vfx::Drawable* drawable) {
    auto present_info = vk::PresentInfoKHR{};
    present_info.setWaitSemaphores(semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable->layer->handle);
    present_info.setPImageIndices(&drawable->index);

    vk::Result result = drawable->layer->context->present_queue.presentKHR(present_info);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        spdlog::info("Swapchain is out of date");
    } else if (result == vk::Result::eSuboptimalKHR) {
        spdlog::info("Swapchain is suboptimal");
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void vfx::CommandBuffer::setPipelineState(const Arc<PipelineState>& state) {
    handle.bindPipeline(vk::PipelineBindPoint::eGraphics, state->pipeline);
}

//void vfx::CommandBuffer::beginRenderPass(const vk::RenderPassBeginInfo& info, vk::SubpassContents contents) {
//    handle.beginRenderPass(info, contents);
//    renderPass = info.renderPass;
//}
//
//void vfx::CommandBuffer::endRenderPass() {
//    renderPass = VK_NULL_HANDLE;
//    handle.endRenderPass();
//}

void vfx::CommandBuffer::beginRendering(const RenderingInfo& description) {
    auto colorAttachmentCount = description.colorAttachments.elements.size();
    if (colorAttachmentCount > colorAttachments.size()) {
        colorAttachments.resize(colorAttachmentCount);
    }

    for (u64 i = 0; i < colorAttachmentCount; ++i) {
        if (retainedReferences) {
            if (description.colorAttachments.elements[i].texture) {
                textureReferences.emplace_back(description.colorAttachments.elements[i].texture);
            }
            if (description.colorAttachments.elements[i].resolveTexture) {
                textureReferences.emplace_back(description.colorAttachments.elements[i].resolveTexture);
            }
        }
        if (description.colorAttachments.elements[i].texture || description.colorAttachments.elements[i].resolveTexture) {
            fillAttachmentInfo(colorAttachments[i], description.colorAttachments.elements[i]);
        }
    }

    vk::RenderingInfo info{};
//    vk_rendering_info.setPNext(info.pNext);
//    vk_rendering_info.setFlags(info.flags);
    info.setRenderArea(description.renderArea);
    info.setLayerCount(description.layerCount);
    info.setViewMask(description.viewMask);
    info.setColorAttachmentCount(u32(colorAttachmentCount));
    info.setPColorAttachments(colorAttachments.data());
    if (retainedReferences) {
        if (description.depthAttachment.texture) {
            textureReferences.emplace_back(description.depthAttachment.texture);
        }
        if (description.depthAttachment.resolveTexture) {
            textureReferences.emplace_back(description.depthAttachment.resolveTexture);
        }
    }
    if (description.depthAttachment.texture || description.depthAttachment.resolveTexture) {
        fillAttachmentInfo(depthAttachment, description.depthAttachment);
        info.setPDepthAttachment(&depthAttachment);
    }
    if (retainedReferences) {
        if (description.stencilAttachment.texture) {
            textureReferences.emplace_back(description.stencilAttachment.texture);
        }
        if (description.stencilAttachment.resolveTexture) {
            textureReferences.emplace_back(description.stencilAttachment.resolveTexture);
        }
    }
    if (description.stencilAttachment.texture || description.stencilAttachment.resolveTexture) {
        fillAttachmentInfo(stencilAttachment, description.stencilAttachment);
        info.setPStencilAttachment(&stencilAttachment);
    }
    handle.beginRendering(info);
}

void vfx::CommandBuffer::endRendering() {
    handle.endRendering();
}

void vfx::CommandBuffer::setScissor(u32 firstScissor, const vk::Rect2D& rect) {
    handle.setScissor(firstScissor, 1, &rect);
}

void vfx::CommandBuffer::setViewport(u32 firstViewport, const vk::Viewport& viewport) {
    handle.setViewport(firstViewport, 1, &viewport);
}

void vfx::CommandBuffer::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
    handle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void vfx::CommandBuffer::drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) {
    handle.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void vfx::CommandBuffer::waitUntilCompleted() {
    std::ignore = commandQueue->context->logical_device.waitForFences(fence, VK_TRUE, std::numeric_limits<u64>::max());
}

void vfx::CommandBuffer::flushBarriers() {
    if (memoryBarriers.empty() && imageMemoryBarriers.empty() && bufferMemoryBarriers.empty()) {
        return;
    }

    vk::DependencyInfo info{};
    info.setMemoryBarriers(memoryBarriers);
    info.setImageMemoryBarriers(imageMemoryBarriers);
    info.setBufferMemoryBarriers(bufferMemoryBarriers);

    handle.pipelineBarrier2(info);

    memoryBarriers.clear();
    imageMemoryBarriers.clear();
    bufferMemoryBarriers.clear();
}

void vfx::CommandBuffer::memoryBarrier(const vk::MemoryBarrier2& barrier) {
    memoryBarriers.emplace_back(barrier);
}

void vfx::CommandBuffer::imageMemoryBarrier(const vk::ImageMemoryBarrier2& barrier) {
    imageMemoryBarriers.emplace_back(barrier);
}

void vfx::CommandBuffer::bufferMemoryBarrier(const vk::BufferMemoryBarrier2& barrier) {
    bufferMemoryBarriers.emplace_back(barrier);
}

void vfx::CommandBuffer::bindIndexBuffer(const Arc<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    if (retainedReferences) {
        bufferReferences.emplace_back(buffer);
    }
    handle.bindIndexBuffer(buffer->handle, offset, indexType);
}

void vfx::CommandBuffer::bindVertexBuffer(int firstBinding, const Arc<Buffer>& buffer, vk::DeviceSize offset) {
    if (retainedReferences) {
        bufferReferences.emplace_back(buffer);
    }
    handle.bindVertexBuffers(firstBinding, 1, &buffer->handle, &offset);
}

vfx::CommandQueue::CommandQueue() {}

vfx::CommandQueue::~CommandQueue() {
    context->freeCommandQueue(this);
}

auto vfx::CommandQueue::makeCommandBuffer() -> vfx::CommandBuffer* {
//    std::ignore = context->logical_device.waitForFences(fences, VK_FALSE, std::numeric_limits<u64>::max());

    while (true) {
        for (auto& commandBuffer : commandBuffers) {
            vk::Result result = context->logical_device.getFenceStatus(commandBuffer.fence);
            if (result == vk::Result::eSuccess) {
                std::ignore = context->logical_device.resetFences(1, &commandBuffer.fence);
                // todo: release references to resources after execution completed
                commandBuffer.releaseReferences();
                commandBuffer.retainedReferences = true;
                return &commandBuffer;
            }
        }
    }

    return nullptr;
}

auto vfx::CommandQueue::makeCommandBufferWithUnretainedReferences() -> vfx::CommandBuffer* {
//    std::ignore = context->logical_device.waitForFences(fences, VK_FALSE, std::numeric_limits<u64>::max());

    while (true) {
        for (auto& commandBuffer : commandBuffers) {
            vk::Result result = context->logical_device.getFenceStatus(commandBuffer.fence);
            if (result == vk::Result::eSuccess) {
                std::ignore = context->logical_device.resetFences(1, &commandBuffer.fence);
                // todo: release references to resources after execution completed
                commandBuffer.releaseReferences();
                commandBuffer.retainedReferences = false;
                return &commandBuffer;
            }
        }
    }

    return nullptr;
}