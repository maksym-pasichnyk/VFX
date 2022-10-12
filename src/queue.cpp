#include "queue.hpp"
#include "pass.hpp"
#include "window.hpp"
#include "context.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "drawable.hpp"
#include "swapchain.hpp"

#include "spdlog/spdlog.h"

void vfx::CommandBuffer::reset() {
    for (auto& [_, pipeline] : pipelines) {
        commandQueue->context->logical_device.destroyPipeline(pipeline);
    }
    pipelines.clear();
}

auto vfx::CommandBuffer::makePipeline(i32 subpass) -> vk::Pipeline {
    auto key = std::make_tuple(pipelineState, renderPass, subpass);
    if (auto it = pipelines.find(key); it != pipelines.end()) {
        return it->second;
    }

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.setViewportCount(1);
    viewportState.setScissorCount(1);

    std::array dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.setDynamicStates(dynamicStates);

    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};

    if (pipelineState->vertexModule) {
        vk::PipelineShaderStageCreateInfo info{};
        info.setStage(vk::ShaderStageFlagBits::eVertex);
        info.setModule(pipelineState->vertexModule);
        info.setPName(pipelineState->description.vertexShader.value().entry.c_str());
        stages.emplace_back(info);
    }

    if (pipelineState->fragmentModule) {
        vk::PipelineShaderStageCreateInfo info{};
        info.setStage(vk::ShaderStageFlagBits::eFragment);
        info.setModule(pipelineState->fragmentModule);
        info.setPName(pipelineState->description.fragmentShader.value().entry.c_str());
        stages.emplace_back(info);
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.setVertexBindingDescriptions(pipelineState->description.bindings);
    vertexInputState.setVertexAttributeDescriptions(pipelineState->description.attributes);

    vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.setAttachments(pipelineState->description.attachments.elements);

    auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{};
    pipeline_create_info.setStages(stages);
    pipeline_create_info.setPVertexInputState(&vertexInputState);
    pipeline_create_info.setPInputAssemblyState(&pipelineState->description.inputAssemblyState);
    pipeline_create_info.setPViewportState(&viewportState);
    pipeline_create_info.setPRasterizationState(&pipelineState->description.rasterizationState);
    pipeline_create_info.setPMultisampleState(&pipelineState->description.multisampleState);
    pipeline_create_info.setPDepthStencilState(&pipelineState->description.depthStencilState);
    pipeline_create_info.setPColorBlendState(&colorBlendState);
    pipeline_create_info.setPDynamicState(&dynamicState);
    pipeline_create_info.setLayout(pipelineState->pipelineLayout);
    pipeline_create_info.setRenderPass(renderPass);
    pipeline_create_info.setSubpass(subpass);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    vk::Pipeline pipeline{};
    auto result = commandQueue->context->logical_device.createGraphicsPipelines(
        {},
        1,
        &pipeline_create_info,
        nullptr,
        &pipeline
    );
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
    spdlog::info("Created pipeline (state = {}, pass = {}, subpass = {})", (void*)pipelineState.get(), (void*)renderPass, subpass);
    pipelines.emplace(key, pipeline);
    return pipeline;
}

void vfx::CommandBuffer::begin(const vk::CommandBufferBeginInfo& info) {
    pipelineState = {};
    handle.begin(info);
}

void vfx::CommandBuffer::end() {
    handle.end();
}

void vfx::CommandBuffer::submit() {
    auto submit_info = vk::SubmitInfo{};
    submit_info.setCommandBuffers(handle);
    submit_info.setSignalSemaphores(semaphore);

    commandQueue->queue.submit(submit_info, fence);
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
    pipelineState = state;
}

void vfx::CommandBuffer::beginRenderPass(const vk::RenderPassBeginInfo& info, vk::SubpassContents contents) {
    handle.beginRenderPass(info, contents);
    renderPass = info.renderPass;
}

void vfx::CommandBuffer::endRenderPass() {
    renderPass = VK_NULL_HANDLE;
    handle.endRenderPass();
}

void vfx::CommandBuffer::beginRendering(const RenderingInfo& info) {

}

void vfx::CommandBuffer::endRendering() {
}

void vfx::CommandBuffer::setScissor(u32 firstScissor, const vk::Rect2D& rect) {
    handle.setScissor(firstScissor, 1, &rect);
}

void vfx::CommandBuffer::setViewport(u32 firstViewport, const vk::Viewport& viewport) {
    handle.setViewport(firstViewport, 1, &viewport);
}

void vfx::CommandBuffer::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
    handle.bindPipeline(vk::PipelineBindPoint::eGraphics, makePipeline(0));
    handle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void vfx::CommandBuffer::drawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) {
    handle.bindPipeline(vk::PipelineBindPoint::eGraphics, makePipeline(0));
    handle.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void vfx::CommandBuffer::waitUntilCompleted() {
    std::ignore = commandQueue->context->logical_device.waitForFences(fence, VK_TRUE, std::numeric_limits<u64>::max());
}

vfx::CommandQueue::CommandQueue() {}

vfx::CommandQueue::~CommandQueue() {
    context->freeCommandQueue(this);
}

auto vfx::CommandQueue::makeCommandBuffer() -> vfx::CommandBuffer* {
    std::ignore = context->logical_device.waitForFences(fences, VK_FALSE, std::numeric_limits<u64>::max());

    for (auto& commandBuffer : commandBuffers) {
        vk::Result result = context->logical_device.getFenceStatus(commandBuffer.fence);
        if (result == vk::Result::eSuccess) {
            std::ignore = context->logical_device.resetFences(1, &commandBuffer.fence);
            return &commandBuffer;
        }
        if (result == vk::Result::eErrorDeviceLost) {
            throw std::runtime_error(vk::to_string(result));
        }
    }
    return nullptr;
}