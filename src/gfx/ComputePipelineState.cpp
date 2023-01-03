#include "ComputePipelineState.hpp"
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

gfx::ComputePipelineState::ComputePipelineState(SharedPtr<Device> device, const SharedPtr<Function>& function) : mDevice(std::move(device)) {
    std::vector<vk::PushConstantRange> push_constant_ranges = {};
    std::vector<DescriptorSetLayoutCreateInfo> descriptor_sets = {};

    for (auto& pcb : std::span(function->mLibrary->mSpvReflectShaderModule.push_constant_blocks, function->mLibrary->mSpvReflectShaderModule.push_constant_block_count)) {
        vk::PushConstantRange push_constant_range = {};
        push_constant_range.setSize(pcb.size);
        push_constant_range.setOffset(pcb.offset);
        push_constant_range.setStageFlags(vk::ShaderStageFlags(function->mEntryPoint->shader_stage));
        push_constant_ranges.emplace_back(push_constant_range);
    }

    for (auto& sds : std::span(function->mLibrary->mSpvReflectShaderModule.descriptor_sets, function->mLibrary->mSpvReflectShaderModule.descriptor_set_count)) {
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

    vkDescriptorSetLayouts.resize(descriptor_sets.size());
    for (uint32_t i = 0; i < descriptor_sets.size(); ++i) {
        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.setBindings(descriptor_sets[i].bindings);

        vkDescriptorSetLayouts[i] = mDevice->vkDevice.createDescriptorSetLayout(descriptor_set_layout_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(vkDescriptorSetLayouts);
    pipeline_layout_create_info.setPushConstantRanges(push_constant_ranges);
    vkPipelineLayout = mDevice->vkDevice.createPipelineLayout(pipeline_layout_create_info, nullptr, mDevice->vkDispatchLoaderDynamic);

    vk::PipelineShaderStageCreateInfo shader_stage_create_info{};
    shader_stage_create_info.setStage(vk::ShaderStageFlagBits::eCompute);
    shader_stage_create_info.setModule(function->mLibrary->vkShaderModule);
    shader_stage_create_info.setPName(function->mFunctionName.c_str());

    vk::ComputePipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.setStage(shader_stage_create_info);
    pipeline_create_info.setLayout(vkPipelineLayout);
    pipeline_create_info.setBasePipelineHandle(nullptr);
    pipeline_create_info.setBasePipelineIndex(0);

    std::ignore = mDevice->vkDevice.createComputePipelines(
        {},
        1,
        &pipeline_create_info,
        nullptr,
        &vkPipeline,
        mDevice->vkDispatchLoaderDynamic
    );
}

gfx::ComputePipelineState::~ComputePipelineState() {
    for (auto& layout : vkDescriptorSetLayouts) {
        mDevice->vkDevice.destroyDescriptorSetLayout(layout, nullptr, mDevice->vkDispatchLoaderDynamic);
    }

    mDevice->vkDevice.destroyPipelineLayout(vkPipelineLayout, nullptr, mDevice->vkDispatchLoaderDynamic);
    mDevice->vkDevice.destroyPipeline(vkPipeline, nullptr, mDevice->vkDispatchLoaderDynamic);
}
