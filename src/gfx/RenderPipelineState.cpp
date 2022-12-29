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
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};
    
    for (auto& function : description.functions) {
        // todo: merge if overlaps
        for (auto& i : std::span(function->mEntryPoint->used_push_constants, function->mEntryPoint->used_push_constant_count)) {
            vk::PushConstantRange push_constant_range = {};
            push_constant_range.setSize(function->mLibrary->mSpvReflectShaderModule.push_constant_blocks[i].size);
            push_constant_range.setOffset(function->mLibrary->mSpvReflectShaderModule.push_constant_blocks[i].offset);
            push_constant_range.setStageFlags(vk::ShaderStageFlags(function->mEntryPoint->shader_stage));
        }

        for (auto& sds : std::span(function->mEntryPoint->descriptor_sets, function->mEntryPoint->descriptor_set_count)) {
            if (sds.set >= descriptor_sets.size()) {
                descriptor_sets.resize(sds.set + 1);
            }

            for (auto& sb : std::span(sds.bindings, sds.binding_count)) {
                vk::DescriptorSetLayoutBinding binding = {};
                binding.setBinding(sb->binding);
                binding.setDescriptorType(vk::DescriptorType(sb->descriptor_type));
                binding.setDescriptorCount(sb->count);
                binding.setStageFlags(vk::ShaderStageFlags(function->mEntryPoint->shader_stage));
                binding.setPImmutableSamplers(nullptr);

                descriptor_sets[sds.set].emplace(binding);
            }
        }
    }

    vkDescriptorSetLayouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < vkDescriptorSetLayouts.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo layout_create_info = {};
        layout_create_info.setBindings(descriptor_sets[i].bindings);
        vkDescriptorSetLayouts[i] = mDevice->vkDevice.createDescriptorSetLayout(layout_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(vkDescriptorSetLayouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    vkPipelineLayout = mDevice->vkDevice.createPipelineLayout(pipeline_layout_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);

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

    vk::PipelineShaderStageCreateInfo vertex_stage = {};
    vertex_stage.setStage(vk::ShaderStageFlagBits::eVertex);
    vertex_stage.setModule(description.functions[0]->mLibrary->vkShaderModule);
    vertex_stage.setPName(description.functions[0]->mFunctionName.c_str());
    stages.emplace_back(vertex_stage);

    vk::PipelineShaderStageCreateInfo fragment_stage = {};
    fragment_stage.setStage(vk::ShaderStageFlagBits::eFragment);
    fragment_stage.setModule(description.functions[1]->mLibrary->vkShaderModule);
    fragment_stage.setPName(description.functions[1]->mFunctionName.c_str());
    stages.emplace_back(fragment_stage);

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
    pipeline_create_info.setRenderPass(nullptr);
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
    for (auto& layout : vkDescriptorSetLayouts) {
        mDevice->vkDevice.destroyDescriptorSetLayout(layout, nullptr, mDevice->vkDispatchLoaderDynamic);
    }
    mDevice->vkDevice.destroyPipelineLayout(vkPipelineLayout, nullptr, mDevice->vkDispatchLoaderDynamic);
    mDevice->vkDevice.destroyPipeline(vkPipeline, nullptr, mDevice->vkDispatchLoaderDynamic);
}