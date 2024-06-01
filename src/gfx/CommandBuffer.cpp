#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "Drawable.hpp"
#include "Swapchain.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "RenderPipelineState.hpp"
#include "ComputePipelineState.hpp"

#include "spdlog/spdlog.h"

#include "vulkan/vulkan_hash.hpp"

template<typename Fn>
struct Lazy : Fn {
    using Fn::operator();

    operator auto() {
        return (*this)();
    }
};

template<typename Fn>
Lazy(Fn) -> Lazy<Fn>;

gfx::CommandBuffer::CommandBuffer(rc<Device> const& device, rc<CommandQueue> const& queue) : device(device), queue(queue) {
    vk::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.setCommandPool(queue->handle);
    allocate_info.setLevel(vk::CommandBufferLevel::ePrimary);
    allocate_info.setCommandBufferCount(1);
    vk::resultCheck(device->handle.allocateCommandBuffers(&allocate_info, &handle, device->dispatcher), "Failed to allocate command buffer");

    vk::FenceCreateInfo fence_create_info = {};
    vk::resultCheck(device->handle.createFence(&fence_create_info, nullptr, &fence, device->dispatcher), "Failed to create fence");

    vk::SemaphoreCreateInfo semaphore_create_info = {};
    vk::resultCheck(device->handle.createSemaphore(&semaphore_create_info, nullptr, &semaphore, device->dispatcher), "Failed to create semaphore");

//    vk::DescriptorPoolCreateInfo pool_create_info = {};
//    pool_create_info.setMaxSets(1024);
//    pool_create_info.setPoolSizes(pool_sizes);
//    auto pool = raii.handle.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, raii.dispatcher);
}

gfx::CommandBuffer::~CommandBuffer() {
    device->handle.destroySemaphore(semaphore, nullptr, device->dispatcher);
    device->handle.destroyFence(fence, nullptr, device->dispatcher);

    for (auto& value : descriptor_pools) {
        device->handle.destroyDescriptorPool(value, nullptr, device->dispatcher);
    }
}

void gfx::CommandBuffer::begin(vk::CommandBufferBeginInfo const& begin_info) {
    handle.begin(begin_info, device->dispatcher);

    for (auto& value : descriptor_pools) {
        device->handle.resetDescriptorPool(value, {}, device->dispatcher);
    }
}

void gfx::CommandBuffer::end() {
    handle.end(device->dispatcher);
}

void gfx::CommandBuffer::submit() {
    vk::SubmitInfo submit_info = {};
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&handle);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&semaphore);

    vk::resultCheck(device->handle.resetFences(1, &fence, device->dispatcher), "Failed to reset fence");

    // todo: get valid device for submit
    device->handle.getQueue(0, 0, device->dispatcher).submit(submit_info, fence, device->dispatcher);
}

void gfx::CommandBuffer::present(rc<gfx::Drawable> const& drawable) {
    vk::PresentInfoKHR present_info = {};
    present_info.setWaitSemaphores(semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable->swapchain);
    present_info.setPImageIndices(&drawable->drawableIndex);

    // todo: get valid device for present
    auto result = device->handle.getQueue(0, 0, device->dispatcher).presentKHR(present_info, device->dispatcher);
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error(vk::to_string(result));
    }
}

void gfx::CommandBuffer::waitUntilCompleted() {
    vk::resultCheck(device->handle.waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max(), device->dispatcher), "Failed to wait for fence");
}

void gfx::CommandBuffer::setImageLayout(const rc<Texture>& texture, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlagBits2 srcAccessMask, vk::AccessFlagBits2 dstAccessMask) {
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

    handle.pipelineBarrier2(dependency_info, device->dispatcher);
}

auto gfx::CommandBuffer::newDescriptorSet(const rc<RenderPipelineState>& render_pipeline_state, uint32_t index) -> vk::DescriptorSet {
    for (auto pool : descriptor_pools) {
        vk::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.setDescriptorPool(pool);
        allocate_info.setDescriptorSetCount(1);
        allocate_info.setPSetLayouts(&render_pipeline_state->descriptorSetLayouts[index]);

        vk::DescriptorSet descriptor_set = VK_NULL_HANDLE;
        vk::Result result = device->handle.allocateDescriptorSets(&allocate_info, &descriptor_set, device->dispatcher);
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

    auto pool = device->handle.createDescriptorPool(pool_create_info, VK_NULL_HANDLE, device->dispatcher);
    descriptor_pools.push_back(pool);

    vk::DescriptorSetAllocateInfo allocate_info = {};
    allocate_info.setDescriptorPool(pool);
    allocate_info.setDescriptorSetCount(1);
    allocate_info.setPSetLayouts(&render_pipeline_state->descriptorSetLayouts[index]);

    vk::DescriptorSet descriptor_set = VK_NULL_HANDLE;
    vk::Result result = device->handle.allocateDescriptorSets(&allocate_info, &descriptor_set, device->dispatcher);
    if (result == vk::Result::eSuccess) {
        return descriptor_set;
    }
    return VK_NULL_HANDLE;
}

auto gfx::CommandBuffer::newRenderCommandEncoder(const RenderingInfo& info) -> rc<RenderCommandEncoder> {
    auto encoder = rc<RenderCommandEncoder>(new RenderCommandEncoder(shared_from_this()));
    encoder->_beginRendering(info);
    return encoder;
}

auto gfx::CommandBuffer::newComputeCommandEncoder() -> rc<ComputeCommandEncoder> {
    // todo: create compute command encoder
    return {};
}

gfx::RenderCommandEncoder::RenderCommandEncoder(const rc<CommandBuffer>& commandBuffer) : commandBuffer(commandBuffer) {
    depthClampEnable_           = false;
    rasterizerDiscardEnable_    = false;
    polygonMode_                = vk::PolygonMode::eFill;
    lineWidth_                  = 1.0F;
    cullMode_                   = vk::CullModeFlagBits::eNone;
    frontFace_                  = vk::FrontFace::eClockwise;
    depthBiasEnable_            = false;
    depthBiasConstantFactor_    = 0.0F;
    depthBiasClamp_             = 0.0F;
    depthBiasSlopeFactor_       = 0.0F;
}

void gfx::RenderCommandEncoder::_beginRendering(const RenderingInfo& info) {
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

    commandBuffer->handle.beginRendering(rendering_info, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::_endRendering() {
    commandBuffer->handle.endRendering(commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::_setup() {
    if ((flags_ & RenderCommandEncoderPipeline) != RenderCommandEncoderPipeline) {
        return;
    }
    flags_ &= ~RenderCommandEncoderPipeline;

    std::size_t key = 0;
    VULKAN_HPP_HASH_COMBINE(key, depthStencilState_.get());
    VULKAN_HPP_HASH_COMBINE(key, renderPipelineState_.get());
    VULKAN_HPP_HASH_COMBINE(key, depthClampEnable_);
    VULKAN_HPP_HASH_COMBINE(key, rasterizerDiscardEnable_);
    VULKAN_HPP_HASH_COMBINE(key, polygonMode_);
    VULKAN_HPP_HASH_COMBINE(key, lineWidth_);
    VULKAN_HPP_HASH_COMBINE(key, cullMode_);
    VULKAN_HPP_HASH_COMBINE(key, frontFace_);
    VULKAN_HPP_HASH_COMBINE(key, depthBiasEnable_);
    VULKAN_HPP_HASH_COMBINE(key, depthBiasConstantFactor_);
    VULKAN_HPP_HASH_COMBINE(key, depthBiasClamp_);
    VULKAN_HPP_HASH_COMBINE(key, depthBiasSlopeFactor_);

    auto it = renderPipelineState_->pipelines.emplace(key, Lazy{[&] -> vk::Pipeline {
        auto renderPipelineStateDescription = renderPipelineState_->description;
        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};

        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
        pipelineViewportStateCreateInfo.setViewportCount(1);
        pipelineViewportStateCreateInfo.setScissorCount(1);
        graphicsPipelineCreateInfo.setPViewportState(&pipelineViewportStateCreateInfo);

        vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
        pipelineRenderingCreateInfo.setViewMask(renderPipelineStateDescription->getViewMask());
        pipelineRenderingCreateInfo.setColorAttachmentFormats(renderPipelineStateDescription->colorAttachmentFormats().elements);
        pipelineRenderingCreateInfo.setDepthAttachmentFormat(renderPipelineStateDescription->getDepthAttachmentFormat());
        pipelineRenderingCreateInfo.setStencilAttachmentFormat(renderPipelineStateDescription->getStencilAttachmentFormat());
        graphicsPipelineCreateInfo.setPNext(&pipelineRenderingCreateInfo);

        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
        pipelineInputAssemblyStateCreateInfo.setTopology(renderPipelineStateDescription->getInputPrimitiveTopology());
        pipelineInputAssemblyStateCreateInfo.setPrimitiveRestartEnable(renderPipelineStateDescription->getPrimitiveRestartEnable());
        graphicsPipelineCreateInfo.setPInputAssemblyState(&pipelineInputAssemblyStateCreateInfo);

        vk::PipelineRasterizationStateCreateInfo rasterization_state = {};
        rasterization_state.setDepthClampEnable(depthClampEnable_);
        rasterization_state.setRasterizerDiscardEnable(rasterizerDiscardEnable_);
        rasterization_state.setPolygonMode(polygonMode_);
        rasterization_state.setLineWidth(lineWidth_);
        rasterization_state.setCullMode(cullMode_);
        rasterization_state.setFrontFace(frontFace_);
        rasterization_state.setDepthBiasEnable(depthBiasEnable_);
        rasterization_state.setDepthBiasConstantFactor(depthBiasConstantFactor_);
        rasterization_state.setDepthBiasClamp(depthBiasClamp_);
        rasterization_state.setDepthBiasSlopeFactor(depthBiasSlopeFactor_);

        vk::PipelineTessellationStateCreateInfo tessellation_state = {};
        if (auto tesselationState = renderPipelineStateDescription->getTessellationState()) {
            tessellation_state.setPatchControlPoints(tesselationState->patch_control_points);
        }

        auto rasterization_samples = [&renderPipelineStateDescription] {
            switch (renderPipelineStateDescription->getRasterSampleCount()) {
                case 1: return vk::SampleCountFlagBits::e1;
                case 2: return vk::SampleCountFlagBits::e2;
                case 4: return vk::SampleCountFlagBits::e4;
                case 8: return vk::SampleCountFlagBits::e8;
                case 16: return vk::SampleCountFlagBits::e16;
                case 32: return vk::SampleCountFlagBits::e32;
                case 64: return vk::SampleCountFlagBits::e64;
                default: return vk::SampleCountFlagBits{};
            }
        }();

        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
        pipelineMultisampleStateCreateInfo.setRasterizationSamples(rasterization_samples);
        if (auto multisampleState = renderPipelineStateDescription->getMultisampleState()) {
            pipelineMultisampleStateCreateInfo.setSampleShadingEnable(multisampleState->sample_shading_enable);
            pipelineMultisampleStateCreateInfo.setMinSampleShading(multisampleState->min_sample_shading);
            pipelineMultisampleStateCreateInfo.setPSampleMask(multisampleState->sample_mask);
        }
        pipelineMultisampleStateCreateInfo.setAlphaToCoverageEnable(renderPipelineStateDescription->getIsAlphaToCoverageEnabled());
        pipelineMultisampleStateCreateInfo.setAlphaToOneEnable(renderPipelineStateDescription->getIsAlphaToOneEnabled());
        graphicsPipelineCreateInfo.setPMultisampleState(&pipelineMultisampleStateCreateInfo);

        vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};

        auto dynamicStates = std::array{
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
        pipelineDynamicStateCreateInfo.setDynamicStates(dynamicStates);
        graphicsPipelineCreateInfo.setPDynamicState(&pipelineDynamicStateCreateInfo);

        std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = {};
        if (auto vertexFunction = renderPipelineStateDescription->getVertexFunction()) {
            vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
            pipelineShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);
            pipelineShaderStageCreateInfo.setModule(vertexFunction->library->handle);
            pipelineShaderStageCreateInfo.setPName(vertexFunction->name.c_str());
            pipelineShaderStageCreateInfos.emplace_back(pipelineShaderStageCreateInfo);
        }
        if (auto fragmentFunction = renderPipelineStateDescription->getFragmentFunction()) {
            vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
            pipelineShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eFragment);
            pipelineShaderStageCreateInfo.setModule(fragmentFunction->library->handle);
            pipelineShaderStageCreateInfo.setPName(fragmentFunction->name.c_str());
            pipelineShaderStageCreateInfos.emplace_back(pipelineShaderStageCreateInfo);
        }
        graphicsPipelineCreateInfo.setStages(pipelineShaderStageCreateInfos);

        vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
        if (auto vertexInputState = renderPipelineStateDescription->getVertexInputState()) {
            pipelineVertexInputStateCreateInfo.setVertexBindingDescriptions(vertexInputState->bindings);
            pipelineVertexInputStateCreateInfo.setVertexAttributeDescriptions(vertexInputState->attributes);
        }
        graphicsPipelineCreateInfo.setPVertexInputState(&pipelineVertexInputStateCreateInfo);

        vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
        pipelineColorBlendStateCreateInfo.setAttachments(renderPipelineStateDescription->colorBlendAttachments().elements);
        graphicsPipelineCreateInfo.setPColorBlendState(&pipelineColorBlendStateCreateInfo);

        graphicsPipelineCreateInfo.setPRasterizationState(&rasterization_state);
        graphicsPipelineCreateInfo.setPTessellationState(&tessellation_state);

        if (depthStencilState_) {
            pipelineDepthStencilStateCreateInfo.setDepthTestEnable(depthStencilState_->isDepthTestEnabled);
            pipelineDepthStencilStateCreateInfo.setDepthWriteEnable(depthStencilState_->isDepthWriteEnabled);
            pipelineDepthStencilStateCreateInfo.setDepthCompareOp(depthStencilState_->depthCompareFunction);
            pipelineDepthStencilStateCreateInfo.setDepthBoundsTestEnable(depthStencilState_->isDepthBoundsTestEnabled);
            pipelineDepthStencilStateCreateInfo.setMinDepthBounds(depthStencilState_->minDepthBounds);
            pipelineDepthStencilStateCreateInfo.setMaxDepthBounds(depthStencilState_->maxDepthBounds);
            pipelineDepthStencilStateCreateInfo.setStencilTestEnable(depthStencilState_->isStencilTestEnabled);
            pipelineDepthStencilStateCreateInfo.setFront(depthStencilState_->frontFaceStencil);
            pipelineDepthStencilStateCreateInfo.setBack(depthStencilState_->backFaceStencil);

            graphicsPipelineCreateInfo.setPDepthStencilState(&pipelineDepthStencilStateCreateInfo);
        }

        graphicsPipelineCreateInfo.setLayout(renderPipelineState_->pipelineLayout);
        graphicsPipelineCreateInfo.setRenderPass(nullptr);
        graphicsPipelineCreateInfo.setSubpass(0);
        graphicsPipelineCreateInfo.setBasePipelineHandle(nullptr);
        graphicsPipelineCreateInfo.setBasePipelineIndex(0);

        vk::Pipeline pipeline;
        vk::resultCheck(commandBuffer->device->handle.createGraphicsPipelines(renderPipelineState_->pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline, commandBuffer->device->dispatcher), "Failed to create graphics pipeline");
        return pipeline;
    }});
    commandBuffer->handle.bindPipeline(vk::PipelineBindPoint::eGraphics, it.first->second, commandBuffer->device->dispatcher);
}

auto gfx::RenderCommandEncoder::getCommandBuffer() -> rc<CommandBuffer> {
    return commandBuffer;
}

void gfx::RenderCommandEncoder::endEncoding() {
    _endRendering();
}

void gfx::RenderCommandEncoder::setDepthClampEnable(bool depthClampEnable) {
    if (depthClampEnable_ != depthClampEnable) {
        flags_ |= RenderCommandEncoderPipeline;
        depthClampEnable_ = depthClampEnable;
    }
}

void gfx::RenderCommandEncoder::setRasterizerDiscardEnable(bool rasterizerDiscardEnable) {
    if (rasterizerDiscardEnable_ != rasterizerDiscardEnable) {
        flags_ |= RenderCommandEncoderPipeline;
        rasterizerDiscardEnable_ = rasterizerDiscardEnable;
    }
}

void gfx::RenderCommandEncoder::setPolygonMode(vk::PolygonMode polygonMode) {
    if (polygonMode_ != polygonMode) {
        flags_ |= RenderCommandEncoderPipeline;
        polygonMode_ = polygonMode;
    }
}

void gfx::RenderCommandEncoder::setLineWidth(float lineWidth) {
    if (lineWidth_ != lineWidth) {
        flags_ |= RenderCommandEncoderPipeline;
        lineWidth_ = lineWidth;
    }
}

void gfx::RenderCommandEncoder::setCullMode(vk::CullModeFlagBits cullMode) {
    if (cullMode_ != cullMode) {
        flags_ |= RenderCommandEncoderPipeline;
        cullMode_ = cullMode;
    }
}

void gfx::RenderCommandEncoder::setFrontFace(vk::FrontFace frontFace) {
    if (frontFace_ != frontFace) {
        flags_ |= RenderCommandEncoderPipeline;
        frontFace_ = frontFace;
    }
}

void gfx::RenderCommandEncoder::setDepthBiasEnable(bool depthBiasEnable) {
    if (depthBiasEnable_ != depthBiasEnable) {
        flags_ |= RenderCommandEncoderPipeline;
        depthBiasEnable_ = depthBiasEnable;
    }
}

void gfx::RenderCommandEncoder::setDepthBiasConstantFactor(float depthBiasConstantFactor) {
    if (depthBiasConstantFactor_ != depthBiasConstantFactor) {
        flags_ |= RenderCommandEncoderPipeline;
        depthBiasConstantFactor_ = depthBiasConstantFactor;
    }
}

void gfx::RenderCommandEncoder::setDepthBiasClamp(float depthBiasClamp) {
    if (depthBiasClamp_ != depthBiasClamp) {
        flags_ |= RenderCommandEncoderPipeline;
        depthBiasClamp_ = depthBiasClamp;
    }
}

void gfx::RenderCommandEncoder::setDepthBiasSlopeFactor(float depthBiasSlopeFactor) {
    if (depthBiasSlopeFactor_ != depthBiasSlopeFactor) {
        flags_ |= RenderCommandEncoderPipeline;
        depthBiasSlopeFactor_ = depthBiasSlopeFactor;
    }
}

void gfx::RenderCommandEncoder::setDepthStencilState(rc<DepthStencilState> depthStencilState) {
    if (depthStencilState_ != depthStencilState) {
        flags_ |= RenderCommandEncoderPipeline;
        depthStencilState_ = std::move(depthStencilState);
    }
}

void gfx::RenderCommandEncoder::setRenderPipelineState(rc<RenderPipelineState> renderPipelineState) {
    if (renderPipelineState_ != renderPipelineState) {
        flags_ |= RenderCommandEncoderPipeline;
        renderPipelineState_ = std::move(renderPipelineState);
    }
}

void gfx::RenderCommandEncoder::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    commandBuffer->handle.setScissor(firstScissor, 1, &rect, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    commandBuffer->handle.setViewport(firstViewport, 1, &viewport, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::bindIndexBuffer(const rc<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    commandBuffer->handle.bindIndexBuffer(buffer->handle, offset, indexType, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::bindVertexBuffer(int firstBinding, const rc<Buffer>& buffer, vk::DeviceSize offset) {
    commandBuffer->handle.bindVertexBuffers(firstBinding, 1, &buffer->handle, &offset, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    _setup();
    commandBuffer->handle.draw(vertexCount, instanceCount, firstVertex, firstInstance, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    _setup();
    commandBuffer->handle.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot) {
    commandBuffer->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, renderPipelineState_->pipelineLayout, slot, 1, &descriptorSet, 0, nullptr, commandBuffer->device->dispatcher);
}

void gfx::RenderCommandEncoder::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    commandBuffer->handle.pushConstants(renderPipelineState_->pipelineLayout, stageFlags, offset, size, data, commandBuffer->device->dispatcher);
}

void gfx::ComputeCommandEncoder::setComputePipelineState(const rc<ComputePipelineState>& state) {
    commandBuffer->handle.bindPipeline(vk::PipelineBindPoint::eCompute, state->pipeline, commandBuffer->device->dispatcher);
}

void gfx::ComputeCommandEncoder::bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot) {
    commandBuffer->handle.bindDescriptorSets(vk::PipelineBindPoint::eCompute, currentPipelineState->pipeline_layout, slot, 1, &descriptorSet, 0, nullptr, commandBuffer->device->dispatcher);
}

void gfx::ComputeCommandEncoder::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    commandBuffer->handle.pushConstants(currentPipelineState->pipeline_layout, stageFlags, offset, size, data, commandBuffer->device->dispatcher);
}

void gfx::ComputeCommandEncoder::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    commandBuffer->handle.dispatch(groupCountX, groupCountY, groupCountZ, commandBuffer->device->dispatcher);
}