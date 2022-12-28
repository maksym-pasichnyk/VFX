#include "ComputePipelineState.hpp"
#include "Function.hpp"
#include "Library.hpp"
#include "Device.hpp"

#include <spirv_reflect.h>

gfx::ComputePipelineState::ComputePipelineState(SharedPtr<Device> device, SharedPtr<Function> function) : mDevice(std::move(device)) {
    struct DescriptorSetDescription {
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};

        void add(const vk::DescriptorSetLayoutBinding& inBinding) {
            for (auto& binding : bindings) {
                if (canMerge(binding, inBinding)) {
                    binding.stageFlags |= inBinding.stageFlags;
                    return;
                }
            }
            bindings.emplace_back(inBinding);
        }

        static auto canMerge(const vk::DescriptorSetLayoutBinding& first, const vk::DescriptorSetLayoutBinding& second) -> bool {
            // todo: check immutable samplers

            return first.binding == second.binding
                && first.descriptorType == second.descriptorType
                && first.descriptorCount == second.descriptorCount;
        }
    };

    std::vector<vk::PushConstantRange> constant_ranges = {};
    std::vector<DescriptorSetDescription> descriptor_set_descriptions = {};

    auto addShaderModule = [&](const SharedPtr<Function>& function) {
        auto stage_flags = vk::ShaderStageFlagBits(function->mLibrary->spvReflectShaderModule->shader_stage);

        auto refl_constant_blocks = std::span(
            function->mLibrary->spvReflectShaderModule->push_constant_blocks,
            function->mLibrary->spvReflectShaderModule->push_constant_block_count
        );

        constant_ranges.reserve(refl_constant_blocks.size());
        for (auto& refl_block : refl_constant_blocks) {
            auto& constant_range = constant_ranges.emplace_back();

            constant_range.setSize(refl_block.size);
            constant_range.setOffset(refl_block.offset);
            constant_range.setStageFlags(stage_flags);
        }

        auto refl_descriptor_sets = std::span(
            function->mLibrary->spvReflectShaderModule->descriptor_sets,
            function->mLibrary->spvReflectShaderModule->descriptor_set_count
        );

        for (auto& refl_set : refl_descriptor_sets) {
            if (refl_set.set >= descriptor_set_descriptions.size()) {
                descriptor_set_descriptions.resize(refl_set.set + 1);
            }

            auto refl_descriptor_bindings = std::span(
                refl_set.bindings,
                refl_set.binding_count
            );

            for (auto& refl_binding : refl_descriptor_bindings) {
                auto binding = vk::DescriptorSetLayoutBinding{
                    .binding = refl_binding->binding,
                    .descriptorType = vk::DescriptorType(refl_binding->descriptor_type),
                    .descriptorCount = refl_binding->count,
                    .stageFlags = stage_flags,
                    .pImmutableSamplers = nullptr
                };
                descriptor_set_descriptions.at(refl_set.set).add(binding);
            }
        }
    };

    addShaderModule(function);

    vkDescriptorSetLayoutArray.resize(descriptor_set_descriptions.size());
    for (uint32_t i = 0; i < vkDescriptorSetLayoutArray.size(); ++i) {
        auto dsl_create_info = vk::DescriptorSetLayoutCreateInfo{};
        dsl_create_info.setBindings(descriptor_set_descriptions[i].bindings);
        vkDescriptorSetLayoutArray[i] = mDevice->vkDevice.createDescriptorSetLayout(dsl_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info = {};
    pipeline_layout_create_info.setSetLayouts(vkDescriptorSetLayoutArray);
    pipeline_layout_create_info.setPushConstantRanges(constant_ranges);
    vkPipelineLayout = mDevice->vkDevice.createPipelineLayout(pipeline_layout_create_info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);

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
    for (auto& layout : vkDescriptorSetLayoutArray) {
        mDevice->vkDevice.destroyDescriptorSetLayout(layout, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    }

    mDevice->vkDevice.destroyPipelineLayout(vkPipelineLayout, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
    mDevice->vkDevice.destroyPipeline(vkPipeline, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}
