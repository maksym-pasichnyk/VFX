#include "RenderPipelineState.hpp"
#include "Function.hpp"
#include "Library.hpp"
#include "Device.hpp"

#include <spirv_reflect.h>

struct DescriptorSetLayoutCreateInfo {
    std::vector<vk::DescriptorSetLayoutBinding> bindings = {};

    void emplace(const vk::DescriptorSetLayoutBinding& other) {
        for (auto& binding : bindings) {
            if (binding.binding != other.binding) {
                continue;
            }
            if (binding.descriptorType != other.descriptorType) {
                continue;
            }
            if (binding.descriptorCount != other.descriptorCount) {
                continue;
            }
            binding.stageFlags |= other.stageFlags;
            return;
        }
        bindings.emplace_back(other);
    }
};

gfx::RenderPipelineState::RenderPipelineState(SharedPtr<Device> device, const RenderPipelineStateDescription& description) : mDevice(std::move(device)) {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> layout_create_info_array = {};

    for (auto function : description.functions) {
        if (!function) {
            continue;
        }

        auto stage_flags = vk::ShaderStageFlagBits(function->mLibrary->spvReflectShaderModule->shader_stage);

        auto spv_push_constant_blocks = std::span(
            function->mLibrary->spvReflectShaderModule->push_constant_blocks,
            function->mLibrary->spvReflectShaderModule->push_constant_block_count
        );

        push_constant_ranges.reserve(spv_push_constant_blocks.size());
        for (auto& spv_push_constant_block : spv_push_constant_blocks) {
            auto& push_constant_range = push_constant_ranges.emplace_back();

            push_constant_range.setSize(spv_push_constant_block.size);
            push_constant_range.setOffset(spv_push_constant_block.offset);
            push_constant_range.setStageFlags(stage_flags);
        }

        auto spv_descriptor_sets = std::span(
            function->mLibrary->spvReflectShaderModule->descriptor_sets,
            function->mLibrary->spvReflectShaderModule->descriptor_set_count
        );

        for (auto& spv_descriptor_set : spv_descriptor_sets) {
            if (spv_descriptor_set.set >= layout_create_info_array.size()) {
                layout_create_info_array.resize(spv_descriptor_set.set + 1);
            }

            auto spv_bindings = std::span(
                spv_descriptor_set.bindings,
                spv_descriptor_set.binding_count
            );

            for (auto& spv_binding : spv_bindings) {
                vk::DescriptorSetLayoutBinding binding = {};
                binding.setBinding(spv_binding->binding);
                binding.setDescriptorType(vk::DescriptorType(spv_binding->descriptor_type));
                binding.setDescriptorCount(spv_binding->count);
                binding.setStageFlags(stage_flags);
                binding.setPImmutableSamplers(nullptr);
                layout_create_info_array.at(spv_descriptor_set.set).emplace(binding);
            }
        }
    }

    vkDescriptorSetLayoutArray.resize(layout_create_info_array.size());
    for (uint32_t i = 0; i < vkDescriptorSetLayoutArray.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.setBindings(layout_create_info_array[i].bindings);
        vkDescriptorSetLayoutArray[i] = mDevice->vkDevice.createDescriptorSetLayout(layout_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(vkDescriptorSetLayoutArray);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    vkPipelineLayout = mDevice->vkDevice.createPipelineLayout(pipeline_layout_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);

    vk::PipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.setViewportCount(1);
    viewport_state.setScissorCount(1);

    auto dynamicStates = std::array{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.setDynamicStates(dynamicStates);

    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};

    if (description.functions[0]) {
        vk::PipelineShaderStageCreateInfo info = {};
        info.setStage(vk::ShaderStageFlagBits::eVertex);
        info.setModule(description.functions[0]->mLibrary->vkShaderModule);
        info.setPName(description.functions[0]->mFunctionName.c_str());
        stages.emplace_back(info);
    }

    if (description.functions[1]) {
        vk::PipelineShaderStageCreateInfo info = {};
        info.setStage(vk::ShaderStageFlagBits::eFragment);
        info.setModule(description.functions[1]->mLibrary->vkShaderModule);
        info.setPName(description.functions[1]->mFunctionName.c_str());
        stages.emplace_back(info);
    }

    vk::PipelineVertexInputStateCreateInfo vertex_input_state = {};
    if (description.vertexDescription.has_value()) {
        vertex_input_state.setVertexBindingDescriptions(description.vertexDescription->layouts.elements);
        vertex_input_state.setVertexAttributeDescriptions(description.vertexDescription->attributes.elements);
    }

    vk::PipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.setAttachments(description.attachments.elements);

    vk::PipelineRenderingCreateInfo rendering = {};
    rendering.setViewMask(description.viewMask);
    rendering.setColorAttachmentFormats(description.colorAttachmentFormats.elements);
    rendering.setDepthAttachmentFormat(description.depthAttachmentFormat);
    rendering.setStencilAttachmentFormat(description.stencilAttachmentFormat);

    vk::GraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setPNext(&rendering);
    pipeline_create_info.setStages(stages);
    pipeline_create_info.setPVertexInputState(&vertex_input_state);
    pipeline_create_info.setPInputAssemblyState(&description.inputAssemblyState);
    pipeline_create_info.setPViewportState(&viewport_state);
    pipeline_create_info.setPRasterizationState(&description.rasterizationState);
    pipeline_create_info.setPMultisampleState(&description.multisampleState);
    pipeline_create_info.setPDepthStencilState(&description.depthStencilState);
    pipeline_create_info.setPColorBlendState(&color_blend_state);
    pipeline_create_info.setPDynamicState(&dynamic_state);
    pipeline_create_info.setLayout(vkPipelineLayout);
    pipeline_create_info.setRenderPass(VK_NULL_HANDLE);
    pipeline_create_info.setSubpass(0);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    std::ignore = mDevice->vkDevice.createGraphicsPipelines(
        {},
        1,
        &pipeline_create_info,
        nullptr,
        &vkPipeline,
        mDevice->vkDispatchLoaderDynamic
    );
}

gfx::RenderPipelineState::~RenderPipelineState() {
    for (auto& layout : vkDescriptorSetLayoutArray) {
        mDevice->vkDevice.destroyDescriptorSetLayout(layout, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    }
    mDevice->vkDevice.destroyPipelineLayout(vkPipelineLayout, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    mDevice->vkDevice.destroyPipeline(vkPipeline, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}