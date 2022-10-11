#include "queue.hpp"
#include "drawable.hpp"
#include "swapchain.hpp"

void vfx::CommandBuffer::clear() {
    for (auto& [_, pipeline] : pipelines) {
        owner->context->logical_device.destroyPipeline(pipeline);
    }
    pipelines.clear();
    pipelineState = {};
}

auto vfx::CommandBuffer::makePipeline(i32 subpass) -> vk::Pipeline {
    auto key = std::make_tuple(pipelineState, renderPass, subpass);
    if (auto it = pipelines.find(key); it != pipelines.end()) {
        return it->second;
    }

    spdlog::info("Created pipeline (state = {}, pass = {}, subpass = {})", (void*)pipelineState.get(), (void*)renderPass, subpass);

    vk::PipelineViewportStateCreateInfo viewportState = {};
    viewportState.viewportCount = 1;
    viewportState.pViewports = nullptr;
    viewportState.scissorCount = 1;
    viewportState.pScissors = nullptr;

    std::array dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.setDynamicStates(dynamicStates);

    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};
    stages.resize(pipelineState->description.shaders.size());
    for (u64 i = 0; i < pipelineState->description.shaders.size(); ++i) {
        stages[i].setStage(pipelineState->description.shaders[i].stage);
        stages[i].setModule(pipelineState->modules[i]);
        stages[i].setPName(pipelineState->description.shaders[i].entry.c_str());
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.setVertexBindingDescriptions(pipelineState->description.bindings);
    vertexInputState.setVertexAttributeDescriptions(pipelineState->description.attributes);

    vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.setAttachments(pipelineState->description.attachments.elements);

    auto pipeline_create_info = vk::GraphicsPipelineCreateInfo{};
    pipeline_create_info.setStages(stages);
    pipeline_create_info.pVertexInputState = &vertexInputState;
    pipeline_create_info.pInputAssemblyState = &pipelineState->description.inputAssemblyState;
    pipeline_create_info.pViewportState = &viewportState;
    pipeline_create_info.pRasterizationState = &pipelineState->description.rasterizationState;
    pipeline_create_info.pMultisampleState = &pipelineState->description.multisampleState;
    pipeline_create_info.pDepthStencilState = &pipelineState->description.depthStencilState;
    pipeline_create_info.pColorBlendState = &colorBlendState;
    pipeline_create_info.pDynamicState = &dynamicState;
    pipeline_create_info.layout = pipelineState->pipelineLayout;
    pipeline_create_info.renderPass = renderPass;
    pipeline_create_info.subpass = subpass;
    pipeline_create_info.basePipelineHandle = nullptr;
    pipeline_create_info.basePipelineIndex = 0;

    vk::Pipeline pipeline{};
    auto result = owner->context->logical_device.createGraphicsPipelines(
        {},
        1,
        &pipeline_create_info,
        nullptr,
        &pipeline
    );
    if (result != vk::Result::eSuccess) {
        spdlog::error("{}", vk::to_string(result));
    }
    pipelines.emplace(key, pipeline);
    return pipeline;
}

void vfx::CommandBuffer::submit() {
    auto submit_info = vk::SubmitInfo{};
    submit_info.setCommandBuffers(handle);
    submit_info.setSignalSemaphores(semaphore);

    owner->queue.submit(submit_info, fence);
}

void vfx::CommandBuffer::present(vfx::Drawable* drawable) {
    auto present_info = vk::PresentInfoKHR{};
    present_info.setWaitSemaphores(semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable->layer->swapchain);
    present_info.setPImageIndices(&drawable->index);

    vk::Result result = drawable->layer->context.present_queue.presentKHR(present_info);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        drawable->layer->rebuild();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swapchain image");
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

void vfx::CommandBuffer::draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
    handle.bindPipeline(vk::PipelineBindPoint::eGraphics, makePipeline(0));
    handle.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

auto vfx::CommandQueue::makeCommandBuffer() -> vfx::CommandBuffer* {
    std::ignore = context->logical_device.waitForFences(fences, VK_FALSE, std::numeric_limits<u64>::max());

    for (u64 i = 0; i < list.size(); ++i) {
        vk::Result result = context->logical_device.getFenceStatus(list[i].fence);
        if (result == vk::Result::eSuccess) {
            std::ignore = context->logical_device.resetFences(1, &list[i].fence);
            return &list[i];
        }
        if (result == vk::Result::eErrorDeviceLost) {
            throw std::runtime_error(vk::to_string(result));
        }
    }
    return nullptr;
}

void vfx::CommandQueue::clearCommandBuffers() {
    for (u64 i = 0; i < list.size(); ++i) {
        list[i].clear();
    }
}