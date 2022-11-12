#pragma once

#include "types.hpp"
#include "spirv_reflect.h"

#include <map>
#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

namespace vfx {
    struct Device;
    struct Function;

    struct Library : std::enable_shared_from_this<Library> {
    public:
        Device* device = {};
        vk::ShaderModule module = {};
        SpvReflectShaderModule reflect = {};

    public:
        Library();
        ~Library();

    public:
        auto makeFunction(std::string name) -> Arc<Function>;
    };

    struct Function {
        Arc<Library> library = {};
        std::string  name = {};
    };

    struct RenderPipelineColorBlendAttachmentStateArray {
        std::vector<vk::PipelineColorBlendAttachmentState> elements{};

        auto operator[](size_t i) -> vk::PipelineColorBlendAttachmentState& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::PipelineColorBlendAttachmentState{});
            }
            return elements[i];
        }
    };

    struct RenderPipelineColorAttachmentFormatArray {
        std::vector<vk::Format> elements = {};

        auto operator[](size_t i) -> vk::Format& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::Format::eUndefined);
            }
            return elements[i];
        }
    };

    struct RenderPipelineVertexLayoutDescriptionArray {
        std::vector<vk::VertexInputBindingDescription> elements = {};

        auto operator[](size_t i) -> vk::VertexInputBindingDescription& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::VertexInputBindingDescription{
                    .binding = u32(i),
                    .inputRate = vk::VertexInputRate::eVertex
                });
            }
            return elements[i];
        }
    };

    struct RenderPipelineVertexAttributeDescriptionArray {
        std::vector<vk::VertexInputAttributeDescription> elements = {};

        auto operator[](size_t i) -> vk::VertexInputAttributeDescription& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::VertexInputAttributeDescription{});
            }
            return elements[i];
        }
    };

    struct RenderPipelineVertexDescription {
        RenderPipelineVertexLayoutDescriptionArray layouts = {};
        RenderPipelineVertexAttributeDescriptionArray attributes = {};
    };

    struct RenderPipelineStateDescription {
        Arc<Function> vertexFunction = {};
        Arc<Function> fragmentFunction = {};

        u32 viewMask = {};
        RenderPipelineColorAttachmentFormatArray colorAttachmentFormats = {};
        vk::Format depthAttachmentFormat   = vk::Format::eUndefined;
        vk::Format stencilAttachmentFormat = vk::Format::eUndefined;

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
        vk::PipelineTessellationStateCreateInfo tessellationState = {};
        vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
        vk::PipelineMultisampleStateCreateInfo multisampleState = {};
        vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
        std::optional<RenderPipelineVertexDescription> vertexDescription = {};
        RenderPipelineColorBlendAttachmentStateArray attachments = {};
    };

    struct RenderPipelineState {
    public:
        Device* device{};
        vk::Pipeline pipeline{};
        vk::PipelineLayout pipelineLayout{};
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};

    public:
        RenderPipelineState();
        ~RenderPipelineState();
    };

    struct ComputePipelineState {
    public:
        Device* device{};
        vk::Pipeline pipeline{};
        vk::PipelineLayout pipelineLayout{};
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};

    public:
        ComputePipelineState();
        ~ComputePipelineState();
    };

    struct Material {

    };
}