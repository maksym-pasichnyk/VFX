#pragma once

#include <map>
#include <vector>
#include <types.hpp>
#include <context.hpp>

struct MaterialDefinition {
    std::map<vk::DescriptorType, u32> pool_sizes_table{};
    std::vector<vk::DescriptorSetLayoutBinding> descriptor_bindings{};

    std::vector<vk::DynamicState> dynamicStates{};
    std::vector<vk::PushConstantRange> constants{};
    std::vector<vk::VertexInputBindingDescription> bindings{};
    std::vector<vk::VertexInputAttributeDescription> attributes{};
    std::vector<vk::PipelineColorBlendAttachmentState> attachments{};

    auto create_pool(Context& context) -> vk::DescriptorPool {
        std::vector<vk::DescriptorPoolSize> pool_sizes{};
        for (auto [type, count] : pool_sizes_table) {
            pool_sizes.emplace_back(vk::DescriptorPoolSize{type, count});
        }
        auto pool_create_info = vk::DescriptorPoolCreateInfo{};
        pool_create_info.setMaxSets(Context::MAX_FRAMES_IN_FLIGHT);
        pool_create_info.setPoolSizes(pool_sizes);
        return context.logical_device.createDescriptorPool(pool_create_info, nullptr);
    }

    auto create_layout(Context& context) -> vk::DescriptorSetLayout {
        auto create_info = vk::DescriptorSetLayoutCreateInfo{};
        create_info.setBindings(descriptor_bindings);
        return context.logical_device.createDescriptorSetLayout(create_info);
    }

    auto create_descriptor(vk::DescriptorType type, u32 count, vk::ShaderStageFlags stage) -> u32 {
        const auto binding = u32(descriptor_bindings.size());
        descriptor_bindings.emplace_back(vk::DescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = count,
            .stageFlags = stage,
            .pImmutableSamplers = nullptr
        });
        pool_sizes_table[type] += Context::MAX_FRAMES_IN_FLIGHT;
        return binding;
    }

    auto create_binding(vk::VertexInputRate rate) -> u32 {
        auto binding = u32(bindings.size());
        bindings.emplace_back(vk::VertexInputBindingDescription{
            .binding   = binding,
            .stride    = 0,
            .inputRate = rate
        });
        return binding;
    }

    auto create_attribute(u32 binding, vk::Format format, u32 size) -> u32 {
        auto location = u32(attributes.size());
        attributes.emplace_back(vk::VertexInputAttributeDescription{
            .location = location,
            .binding  = binding,
            .format   = format,
            .offset   = bindings[binding].stride,
        });
        bindings[binding].stride += size;
        return location;
    }

    auto create_attachment(const vk::PipelineColorBlendAttachmentState& attachment) {
        attachments.emplace_back(attachment);
    }

    auto create_constant(vk::ShaderStageFlags stage, u32 offset, u32 size) {
        constants.emplace_back(vk::PushConstantRange{stage, offset, size});
    }
};
