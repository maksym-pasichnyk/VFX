#pragma once

#include "Device.hpp"
#include "Function.hpp"
#include "DescriptorSet.hpp"

#include <optional>

namespace gfx {
    struct Device;
    struct Function;
    struct BindGroupLayout;

    struct RenderPipelineColorBlendAttachmentStateArray {
        std::vector<vk::PipelineColorBlendAttachmentState> elements = {};

        auto operator[](size_t i) -> vk::PipelineColorBlendAttachmentState& {
            if (elements.size() >= i) {
                elements.resize(i + 1, vk::PipelineColorBlendAttachmentState{
                    .colorWriteMask =
                        vk::ColorComponentFlagBits::eR |
                        vk::ColorComponentFlagBits::eG |
                        vk::ColorComponentFlagBits::eB |
                        vk::ColorComponentFlagBits::eA
                });
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
                    .binding = uint32_t(i),
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
        Function vertexFunction;
        Function fragmentFunction;

        uint32_t viewMask = {};
        RenderPipelineColorAttachmentFormatArray colorAttachmentFormats = {};
        vk::Format depthAttachmentFormat   = vk::Format::eUndefined;
        vk::Format stencilAttachmentFormat = vk::Format::eUndefined;

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = {
            .topology = vk::PrimitiveTopology::eTriangleList
        };
        vk::PipelineTessellationStateCreateInfo tessellationState = {};
        vk::PipelineRasterizationStateCreateInfo rasterizationState = {
            .lineWidth = 1.0f
        };
        vk::PipelineMultisampleStateCreateInfo multisampleState = {};
        vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
        std::optional<RenderPipelineVertexDescription> vertexDescription = {};
        RenderPipelineColorBlendAttachmentStateArray attachments = {};
    };

    struct RenderPipelineStateShared {
        Device device;
        vk::Pipeline pipeline;
        vk::PipelineLayout pipeline_layout;
        std::vector<vk::DescriptorSetLayout> bind_group_layouts;

        explicit RenderPipelineStateShared(Device device);
        ~RenderPipelineStateShared();
    };

    struct RenderPipelineState final {
        std::shared_ptr<RenderPipelineStateShared> shared;

        explicit RenderPipelineState() : shared(nullptr) {}
        explicit RenderPipelineState(std::shared_ptr<RenderPipelineStateShared> shared) : shared(std::move(shared)) {}
    };
}