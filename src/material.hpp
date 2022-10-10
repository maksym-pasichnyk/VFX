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

    struct PipelineColorBlendAttachmentStateArray {
        std::vector<vk::PipelineColorBlendAttachmentState> elements{};

        auto operator[](size_t i) -> vk::PipelineColorBlendAttachmentState& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::PipelineColorBlendAttachmentState{});
            }
            return elements[i];
        }
    };

    struct MaterialDescription {
        std::vector<ShaderDescription> shaders = {};
        std::optional<vk::PipelineRenderingCreateInfo> rendering = {};

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
        PipelineColorBlendAttachmentStateArray attachments = {};
    };

    struct Material {
        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;
//        vk::PipelineBindPoint pipeline_bind_point;

        // todo: bindless
        vk::DescriptorPool descriptor_pool;
        std::vector<vk::DescriptorSet> descriptor_sets{};
        std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{};
    };
}