#pragma once

#include "types.hpp"
#include "spirv_reflect.h"

#include <map>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Context;
    struct Function {
    public:
        Context*         context = {};
        vk::ShaderModule module = {};
        std::string      name = {};

        SpvReflectShaderModule reflect = {};

    public:
        Function();
        ~Function();
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
        Arc<Function> vertexFunction = {};
        Arc<Function> fragmentFunction = {};

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

    struct PipelineState {
    public:
        Context* context{};
        PipelineStateDescription description{};

        vk::PipelineLayout pipelineLayout{};
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};

    public:
        PipelineState();
        ~PipelineState();
    };

    struct Material {

    };
}