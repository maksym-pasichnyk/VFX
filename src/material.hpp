#pragma once

#include "types.hpp"

#include <map>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct ShaderDescription {
        std::vector<char> bytes;
        std::string entry;
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

    struct PipelineStateDescription {
        std::optional<ShaderDescription> vertexShader = {};
        std::optional<ShaderDescription> fragmentShader = {};

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

    struct Context;
    struct PipelineState {
    public:
        Context* context{};
        PipelineStateDescription description{};

        vk::ShaderModule vertexModule = VK_NULL_HANDLE;
        vk::ShaderModule fragmentModule = VK_NULL_HANDLE;

        vk::PipelineLayout pipelineLayout{};
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};

    public:
        PipelineState();
        ~PipelineState();
    };

    struct Material {

    };
}