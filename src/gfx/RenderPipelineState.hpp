#pragma once

#include "Object.hpp"
#include "Function.hpp"

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace gfx {
    struct Device;
    struct Function;

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
        SharedPtr<Function> functions[2] = {};

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

    public:
        void setVertexFunction(SharedPtr<Function> function) {
            functions[0] = std::move(function);
        }

        void setFragmentFunction(SharedPtr<Function> function) {
            functions[1] = std::move(function);
        }
    };

    struct RenderPipelineState final : Referencing {
        friend Device;

    public:
        SharedPtr<Device> mDevice;
        vk::Pipeline vkPipeline = {};
        vk::PipelineLayout vkPipelineLayout = {};
        std::vector<vk::DescriptorSetLayout> vkDescriptorSetLayouts = {};

    private:
        explicit RenderPipelineState(SharedPtr<Device> device, const RenderPipelineStateDescription& description);
        ~RenderPipelineState() override;
    };
}