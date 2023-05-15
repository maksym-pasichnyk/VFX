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

gfx::CommandBuffer::CommandBuffer(const ManagedShared<Device>& device, const ManagedShared<CommandQueue>& queue) : device(device), queue(queue) {
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

auto gfx::CommandBuffer::newDescriptorSet(const ManagedShared<RenderPipelineState>& render_pipeline_state, uint32_t index) -> vk::DescriptorSet {
    for (auto pool : descriptor_pools) {
        vk::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.setDescriptorPool(pool);
        allocate_info.setDescriptorSetCount(1);
        allocate_info.setPSetLayouts(&render_pipeline_state->bindGroupLayouts[index]);

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
    allocate_info.setPSetLayouts(&render_pipeline_state->bindGroupLayouts[index]);

    vk::DescriptorSet descriptor_set = VK_NULL_HANDLE;
    vk::Result result = device->raii.raw.allocateDescriptorSets(&allocate_info, &descriptor_set, device->raii.dispatcher);
    if (result == vk::Result::eSuccess) {
        return descriptor_set;
    }
    return VK_NULL_HANDLE;
}

auto gfx::CommandBuffer::newRenderCommandEncoder(const RenderingInfo& info) -> ManagedShared<RenderCommandEncoder> {
    auto encoder = MakeShared<RenderCommandEncoder>(shared_from_this());
    encoder->_beginRendering(info);
    return encoder;
}

auto gfx::CommandBuffer::newComputeCommandEncoder() -> ManagedShared<ComputeCommandEncoder> {
    // todo: create compute command encoder
    return {};
}

gfx::RenderCommandEncoder::RenderCommandEncoder(const ManagedShared<CommandBuffer>& commandBuffer) : commandBuffer(commandBuffer) {
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

    commandBuffer->raw.beginRendering(rendering_info, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::_endRendering() {
    commandBuffer->raw.endRendering(commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::_setup() {
    if ((flags & RenderCommandEncoderPipeline) == RenderCommandEncoderPipeline) {
        flags &= ~RenderCommandEncoderPipeline;

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

        auto it = renderPipelineState_->pipelines.emplace(key, Lazy{
            [&]() -> vk::Pipeline {
                vk::PipelineViewportStateCreateInfo viewport_state = {};
                viewport_state.setViewportCount(1);
                viewport_state.setScissorCount(1);

                vk::PipelineInputAssemblyStateCreateInfo input_assembly_state = {};
                input_assembly_state.setTopology(renderPipelineState_->inputPrimitiveTopology);
                input_assembly_state.setPrimitiveRestartEnable(renderPipelineState_->primitiveRestartEnable);

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
                if (renderPipelineState_->tessellationState) {
                    tessellation_state.setPatchControlPoints(renderPipelineState_->tessellationState->patch_control_points);
                }

                vk::PipelineMultisampleStateCreateInfo multisample_state = {};
                if (renderPipelineState_->multisampleState) {
                    multisample_state.setRasterizationSamples(renderPipelineState_->multisampleState->rasterization_samples);
                    multisample_state.setSampleShadingEnable(renderPipelineState_->multisampleState->sample_shading_enable);
                    multisample_state.setMinSampleShading(renderPipelineState_->multisampleState->min_sample_shading);
                    multisample_state.setPSampleMask(renderPipelineState_->multisampleState->sample_mask);
                    multisample_state.setAlphaToCoverageEnable(renderPipelineState_->isAlphaToCoverageEnabled);
                    multisample_state.setAlphaToOneEnable(renderPipelineState_->isAlphaToOneEnabled);
                }

                vk::PipelineDepthStencilStateCreateInfo depth_stencil_state = {};

                auto dynamicStates = std::array{
                    vk::DynamicState::eViewport,
                    vk::DynamicState::eScissor
                };

                vk::PipelineDynamicStateCreateInfo dynamic_state = {};
                dynamic_state.setDynamicStates(dynamicStates);

                std::vector<vk::PipelineShaderStageCreateInfo> stages = {};

                vk::PipelineShaderStageCreateInfo vertex_stage = {};
                vertex_stage.setStage(vk::ShaderStageFlagBits::eVertex);
                vertex_stage.setModule(renderPipelineState_->vertexFunction->library->raw);
                vertex_stage.setPName(renderPipelineState_->vertexFunction->name.c_str());
                stages.emplace_back(vertex_stage);

                vk::PipelineShaderStageCreateInfo fragment_stage = {};
                fragment_stage.setStage(vk::ShaderStageFlagBits::eFragment);
                fragment_stage.setModule(renderPipelineState_->fragmentFunction->library->raw);
                fragment_stage.setPName(renderPipelineState_->fragmentFunction->name.c_str());
                stages.emplace_back(fragment_stage);

                vk::PipelineVertexInputStateCreateInfo vertex_input_state = {};
                vertex_input_state.setVertexBindingDescriptions(renderPipelineState_->vertexInputState.bindings);
                vertex_input_state.setVertexAttributeDescriptions(renderPipelineState_->vertexInputState.attributes);

                vk::PipelineColorBlendStateCreateInfo color_blend_state = {};
                color_blend_state.setAttachments(renderPipelineState_->colorBlendAttachments.elements);

                vk::PipelineRenderingCreateInfo rendering = {};
                rendering.setViewMask(renderPipelineState_->viewMask);
                rendering.setColorAttachmentFormats(renderPipelineState_->colorAttachmentFormats.elements);
                rendering.setDepthAttachmentFormat(renderPipelineState_->depthAttachmentFormat);
                rendering.setStencilAttachmentFormat(renderPipelineState_->stencilAttachmentFormat);

                vk::GraphicsPipelineCreateInfo pipeline_create_info = {};
                pipeline_create_info.setPNext(&rendering);
                pipeline_create_info.setStages(stages);
                pipeline_create_info.setPVertexInputState(&vertex_input_state);
                pipeline_create_info.setPInputAssemblyState(&input_assembly_state);
                pipeline_create_info.setPViewportState(&viewport_state);
                pipeline_create_info.setPRasterizationState(&rasterization_state);
                pipeline_create_info.setPTessellationState(&tessellation_state);
                pipeline_create_info.setPMultisampleState(&multisample_state);

                if (depthStencilState_) {
                    depth_stencil_state.setDepthTestEnable(depthStencilState_->depth_test_enable);
                    depth_stencil_state.setDepthWriteEnable(depthStencilState_->depth_write_enable);
                    depth_stencil_state.setDepthCompareOp(depthStencilState_->depth_compare_op);
                    depth_stencil_state.setDepthBoundsTestEnable(depthStencilState_->depth_bounds_test_enable);
                    depth_stencil_state.setMinDepthBounds(depthStencilState_->min_depth_bounds);
                    depth_stencil_state.setMaxDepthBounds(depthStencilState_->max_depth_bounds);
                    depth_stencil_state.setStencilTestEnable(depthStencilState_->stencil_test_enable);
                    depth_stencil_state.setFront(depthStencilState_->front);
                    depth_stencil_state.setBack(depthStencilState_->back);

                    pipeline_create_info.setPDepthStencilState(&depth_stencil_state);
                }

                pipeline_create_info.setPColorBlendState(&color_blend_state);
                pipeline_create_info.setPDynamicState(&dynamic_state);
                pipeline_create_info.setLayout(renderPipelineState_->pipelineLayout);
                pipeline_create_info.setRenderPass(nullptr);
                pipeline_create_info.setSubpass(0);
                pipeline_create_info.setBasePipelineHandle(nullptr);
                pipeline_create_info.setBasePipelineIndex(0);

                vk::Pipeline pipeline;
                vk::resultCheck(commandBuffer->device->raii.raw.createGraphicsPipelines(renderPipelineState_->cache, 1, &pipeline_create_info, nullptr, &pipeline, commandBuffer->device->raii.dispatcher), "Failed to create graphics pipeline");
                return pipeline;
            }
        });

        auto pipeline = it.first->second;

        commandBuffer->raw.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, commandBuffer->device->raii.dispatcher);
    }
}

auto gfx::RenderCommandEncoder::getCommandBuffer() -> ManagedShared<CommandBuffer> {
    return commandBuffer;
}

void gfx::RenderCommandEncoder::endEncoding() {
    _endRendering();
}

void gfx::RenderCommandEncoder::setDepthClampEnable(bool depthClampEnable) {
    flags |= RenderCommandEncoderPipeline;
    depthClampEnable_ = depthClampEnable;
}

void gfx::RenderCommandEncoder::setRasterizerDiscardEnable(bool rasterizerDiscardEnable) {
    flags |= RenderCommandEncoderPipeline;
    rasterizerDiscardEnable_ = rasterizerDiscardEnable;
}

void gfx::RenderCommandEncoder::setPolygonMode(vk::PolygonMode polygonMode) {
    flags |= RenderCommandEncoderPipeline;
    polygonMode_ = polygonMode;
}

void gfx::RenderCommandEncoder::setLineWidth(float lineWidth) {
    flags |= RenderCommandEncoderPipeline;
    lineWidth_ = lineWidth;
}

void gfx::RenderCommandEncoder::setCullMode(vk::CullModeFlagBits cullMode) {
    flags |= RenderCommandEncoderPipeline;
    cullMode_ = cullMode;
}

void gfx::RenderCommandEncoder::setFrontFace(vk::FrontFace frontFace) {
    flags |= RenderCommandEncoderPipeline;
    frontFace_ = frontFace;
}

void gfx::RenderCommandEncoder::setDepthBiasEnable(bool depthBiasEnable) {
    flags |= RenderCommandEncoderPipeline;
    depthBiasEnable_ = depthBiasEnable;
}

void gfx::RenderCommandEncoder::setDepthBiasConstantFactor(float depthBiasConstantFactor) {
    flags |= RenderCommandEncoderPipeline;
    depthBiasConstantFactor_ = depthBiasConstantFactor;
}

void gfx::RenderCommandEncoder::setDepthBiasClamp(float depthBiasClamp) {
    flags |= RenderCommandEncoderPipeline;
    depthBiasClamp_ = depthBiasClamp;
}

void gfx::RenderCommandEncoder::setDepthBiasSlopeFactor(float depthBiasSlopeFactor) {
    flags |= RenderCommandEncoderPipeline;
    depthBiasSlopeFactor_ = depthBiasSlopeFactor;
}

void gfx::RenderCommandEncoder::setDepthStencilState(ManagedShared<DepthStencilState> depthStencilState) {
    flags |= RenderCommandEncoderPipeline;
    depthStencilState_ = std::move(depthStencilState);
}

void gfx::RenderCommandEncoder::setRenderPipelineState(ManagedShared<RenderPipelineState> renderPipelineState) {
    flags |= RenderCommandEncoderPipeline;
    renderPipelineState_ = std::move(renderPipelineState);
}

void gfx::RenderCommandEncoder::setScissor(uint32_t firstScissor, const vk::Rect2D& rect) {
    commandBuffer->raw.setScissor(firstScissor, 1, &rect, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::setViewport(uint32_t firstViewport, const vk::Viewport& viewport) {
    commandBuffer->raw.setViewport(firstViewport, 1, &viewport, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::bindIndexBuffer(const ManagedShared<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType) {
    commandBuffer->raw.bindIndexBuffer(buffer->raw, offset, indexType, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::bindVertexBuffer(int firstBinding, const ManagedShared<Buffer>& buffer, vk::DeviceSize offset) {
    commandBuffer->raw.bindVertexBuffers(firstBinding, 1, &buffer->raw, &offset, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    _setup();
    commandBuffer->raw.draw(vertexCount, instanceCount, firstVertex, firstInstance, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    _setup();
    commandBuffer->raw.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot) {
    commandBuffer->raw.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, renderPipelineState_->pipelineLayout, slot, 1, &descriptorSet, 0, nullptr, commandBuffer->device->raii.dispatcher);
}

void gfx::RenderCommandEncoder::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    commandBuffer->raw.pushConstants(renderPipelineState_->pipelineLayout, stageFlags, offset, size, data, commandBuffer->device->raii.dispatcher);
}

void gfx::ComputeCommandEncoder::setComputePipelineState(const ManagedShared<ComputePipelineState>& state) {
    commandBuffer->raw.bindPipeline(vk::PipelineBindPoint::eCompute, state->pipeline, commandBuffer->device->raii.dispatcher);
}

void gfx::ComputeCommandEncoder::bindDescriptorSet(vk::DescriptorSet descriptorSet, uint32_t slot) {
    commandBuffer->raw.bindDescriptorSets(vk::PipelineBindPoint::eCompute, currentPipelineState->pipeline_layout, slot, 1, &descriptorSet, 0, nullptr, commandBuffer->device->raii.dispatcher);
}

void gfx::ComputeCommandEncoder::pushConstants(vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* data) {
    commandBuffer->raw.pushConstants(currentPipelineState->pipeline_layout, stageFlags, offset, size, data, commandBuffer->device->raii.dispatcher);
}

void gfx::ComputeCommandEncoder::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    commandBuffer->raw.dispatch(groupCountX, groupCountY, groupCountZ, commandBuffer->device->raii.dispatcher);
}