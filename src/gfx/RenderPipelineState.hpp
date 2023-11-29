#pragma once

#include "Device.hpp"
#include "Function.hpp"

#include <map>
#include <optional>

namespace gfx {
    struct CommandBuffer;
    struct RenderCommandEncoder;

    struct RenderPipelineColorBlendAttachmentStateArray {
        std::vector<vk::PipelineColorBlendAttachmentState> elements = {};

        auto operator[](size_t i) -> vk::PipelineColorBlendAttachmentState&;
    };

    struct RenderPipelineColorAttachmentFormatArray {
        std::vector<vk::Format> elements = {};

        auto operator[](size_t i) -> vk::Format&;
    };

    struct TessellationState : public ManagedObject {
        uint32_t patch_control_points = 3;
    };

    struct MultisampleState : public ManagedObject {
        bool                            sample_shading_enable       = false;
        float                           min_sample_shading          = 1.0F;
        vk::Optional<vk::SampleMask>    sample_mask                 = nullptr;
    };

    struct DepthStencilStateDescription {
        bool                    isDepthTestEnabled          = false;
        bool                    isDepthWriteEnabled         = false;
        vk::CompareOp           depthCompareFunction        = vk::CompareOp::eAlways;
        bool                    depth_bounds_test_enable    = false;
        bool                    stencil_test_enable         = false;
        vk::StencilOpState      frontFaceStencil            = {};
        vk::StencilOpState      backFaceStencil             = {};
        float                   min_depth_bounds            = 0.0F;
        float                   max_depth_bounds            = 1.0F;
    };

    struct VertexInputState {
        std::vector<vk::VertexInputBindingDescription>      bindings    = {};
        std::vector<vk::VertexInputAttributeDescription>    attributes  = {};
    };

    struct RenderPipelineStateDescription {
        rc<Function>                         vertexFunction              = {};
        rc<Function>                         fragmentFunction            = {};
        rc<TessellationState>                tessellationState           = MakeShared<TessellationState>();
        rc<MultisampleState>                 multisampleState            = MakeShared<MultisampleState>();
        VertexInputState                                vertexInputState            = {};
        uint32_t                                        viewMask                    = {};
        vk::Format                                      depthAttachmentFormat       = vk::Format::eUndefined;
        vk::Format                                      stencilAttachmentFormat     = vk::Format::eUndefined;
        RenderPipelineColorAttachmentFormatArray        colorAttachmentFormats      = {};
        RenderPipelineColorBlendAttachmentStateArray    colorBlendAttachments       = {};


        // InputAssemblyState
        vk::PrimitiveTopology                           inputPrimitiveTopology      = vk::PrimitiveTopology::eTriangleList;
        bool                                            primitiveRestartEnable      = {};

        // MultisampleState
        uint32_t                                        rasterSampleCount           = 1;
        bool                                            isAlphaToCoverageEnabled    = {};
        bool                                            isAlphaToOneEnabled         = {};
    };

    class DepthStencilState : public ManagedObject {
        friend Device;
        friend RenderCommandEncoder;

    private:
        bool                isDepthTestEnabled      = {};
        bool                isDepthWriteEnabled     = {};
        vk::CompareOp       depthCompareFunction    = {};
        bool                isDepthBoundsTestEnabled   = {};
        bool                isStencilTestEnabled       = {};
        vk::StencilOpState  frontFaceStencil        = {};
        vk::StencilOpState  backFaceStencil         = {};
        float               minDepthBounds          = {};
        float               maxDepthBounds          = {};
    };

    class RenderPipelineState : public ManagedObject {
        friend Device;
        friend CommandBuffer;
        friend RenderCommandEncoder;

    private:
        rc<Device>                           device                      = {};
        rc<Function>                         vertexFunction              = {};
        rc<Function>                         fragmentFunction            = {};
        rc<TessellationState>                tessellationState           = {};
        rc<MultisampleState>                 multisampleState            = {};
        VertexInputState                                vertexInputState            = {};
        uint32_t                                        viewMask                    = {};
        vk::Format                                      depthAttachmentFormat       = {};
        vk::Format                                      stencilAttachmentFormat     = {};
        RenderPipelineColorAttachmentFormatArray        colorAttachmentFormats      = {};
        RenderPipelineColorBlendAttachmentStateArray    colorBlendAttachments       = {};
        vk::PrimitiveTopology                           inputPrimitiveTopology      = {};
        bool                                            primitiveRestartEnable      = {};
        uint32_t                                        rasterSampleCount           = {};
        bool                                            isAlphaToCoverageEnabled    = {};
        bool                                            isAlphaToOneEnabled         = {};

        vk::PipelineCache                               cache                       = {};
        vk::PipelineLayout                              pipelineLayout              = {};
        std::map<std::size_t, vk::Pipeline>             pipelines                   = {};
        std::vector<vk::DescriptorSetLayout>            bindGroupLayouts            = {};

    public:
        explicit RenderPipelineState(rc<Device> device);
        ~RenderPipelineState() override;
    };
}