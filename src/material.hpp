#pragma once

#include <map>
#include <vector>
#include <types.hpp>

namespace vfx {
    struct ShaderDescription {
        std::vector<char> bytes;
        std::string entry;
        vk::ShaderStageFlagBits stage;
    };

    struct MaterialDescription {
        std::vector<ShaderDescription> shaders = {};
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
        vk::PipelineTessellationStateCreateInfo tessellationState = {};
        vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
        vk::PipelineMultisampleStateCreateInfo multisampleState = {};
        vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
        //    vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
        //    vk::Pipeline basePipelineHandle = {};
        //    i32 basePipelineIndex = {};

        std::vector<vk::VertexInputBindingDescription> bindings = {};
        std::vector<vk::VertexInputAttributeDescription> attributes = {};
        std::vector<vk::PipelineColorBlendAttachmentState> attachments = {};

        auto create_attachment(const vk::PipelineColorBlendAttachmentState& state) -> u32 {
            auto attachment = u32(attachments.size());
            attachments.emplace_back(state);
            return attachment;
        }
    };

    struct Material {
        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;
        vk::PipelineBindPoint pipeline_bind_point;

        // todo: bindless
        vk::DescriptorPool descriptor_pool;
        std::vector<vk::DescriptorSet> descriptor_sets{};
        std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{};
    };
}