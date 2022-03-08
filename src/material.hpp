#pragma once

#include <map>
#include <vector>
#include <types.hpp>

struct MaterialDescription {
    struct Resource {
        u32 binding;
        vk::DescriptorType type;
        vk::ShaderStageFlags stage;
        u32 uniform_buffer_size;
    };

    std::vector<Resource> resources{};
    std::vector<vk::PushConstantRange> constants{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    vk::PipelineTessellationStateCreateInfo tessellationState = {};
    vk::PipelineViewportStateCreateInfo viewportState = {};
    vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
    vk::PipelineMultisampleStateCreateInfo multisampleState = {};
    vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
//    vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
    vk::PipelineLayout layout = {};
//    vk::Pipeline basePipelineHandle = {};
//    i32 basePipelineIndex = {};

    std::vector<vk::DynamicState> dynamic_states = {};
    std::vector<vk::PipelineShaderStageCreateInfo> stages = {};
    std::vector<vk::VertexInputBindingDescription> bindings = {};
    std::vector<vk::VertexInputAttributeDescription> attributes = {};
    std::vector<vk::PipelineColorBlendAttachmentState> attachments = {};

    void create_sampler_resource(u32 binding, vk::ShaderStageFlags stage) {
        resources.emplace_back(Resource{binding, vk::DescriptorType::eCombinedImageSampler, stage, 0});
    }

    void create_uniform_buffer_resource(u32 binding, vk::ShaderStageFlags stage, u32 size) {
        resources.emplace_back(Resource{binding, vk::DescriptorType::eUniformBuffer, stage, size});
    }

    void create_input_attachment_resource(u32 binding, vk::ShaderStageFlags stage) {
        resources.emplace_back(Resource{binding, vk::DescriptorType::eInputAttachment, stage, 0});
    }

    auto create_constant(vk::ShaderStageFlags stage, u32 offset, u32 size) {
        constants.emplace_back(vk::PushConstantRange{stage, offset, size});
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

    auto create_attachment(
        bool blendEnable,
        vk::BlendFactor srcColorBlendFactor = vk::BlendFactor::eZero,
        vk::BlendFactor dstColorBlendFactor = vk::BlendFactor::eZero,
        vk::BlendOp colorBlendOp = vk::BlendOp::eAdd,
        vk::BlendFactor srcAlphaBlendFactor = vk::BlendFactor::eZero,
        vk::BlendFactor dstAlphaBlendFactor = vk::BlendFactor::eZero,
        vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd,
        vk::ColorComponentFlags colorWriteMask = {}
    ) -> u32 {
        auto attachment = u32(attachments.size());
        attachments.emplace_back(vk::PipelineColorBlendAttachmentState{
            .blendEnable = blendEnable ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = srcColorBlendFactor,
            .dstColorBlendFactor = dstColorBlendFactor,
            .colorBlendOp = colorBlendOp,
            .srcAlphaBlendFactor = srcAlphaBlendFactor,
            .dstAlphaBlendFactor = dstAlphaBlendFactor,
            .alphaBlendOp = alphaBlendOp,
            .colorWriteMask = colorWriteMask,
        });
        return attachment;
    }
};