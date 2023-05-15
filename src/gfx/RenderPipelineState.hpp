#pragma once

#include "Device.hpp"
#include "Function.hpp"

#include <map>
#include <optional>

namespace gfx {
    struct RenderPipelineColorBlendAttachmentStateArray {
        std::vector<vk::PipelineColorBlendAttachmentState> elements = {};

        auto operator[](size_t i) -> vk::PipelineColorBlendAttachmentState&;
    };

    struct RenderPipelineColorAttachmentFormatArray {
        std::vector<vk::Format> elements = {};

        auto operator[](size_t i) -> vk::Format&;
    };

    struct TessellationState : ManagedObject<TessellationState> {
        uint32_t patch_control_points = 3;
    };

    struct MultisampleState : ManagedObject<MultisampleState> {
        vk::SampleCountFlagBits         rasterization_samples       = vk::SampleCountFlagBits::e1;
        bool                            sample_shading_enable       = false;
        float                           min_sample_shading          = 1.0F;
        vk::Optional<vk::SampleMask>    sample_mask                 = nullptr;
    };

    struct DepthStencilStateDescription {
        bool                    depth_test_enable           = false;
        bool                    depth_write_enable          = false;
        vk::CompareOp           depth_compare_op            = vk::CompareOp::eAlways;
        bool                    depth_bounds_test_enable    = false;
        bool                    stencil_test_enable         = false;
        vk::StencilOpState      front                       = {};
        vk::StencilOpState      back                        = {};
        float                   min_depth_bounds            = 0.0F;
        float                   max_depth_bounds            = 1.0F;
    };

    struct DepthStencilState : ManagedObject<DepthStencilState> {
        bool                depth_test_enable           = {};
        bool                depth_write_enable          = {};
        vk::CompareOp       depth_compare_op            = {};
        bool                depth_bounds_test_enable    = {};
        bool                stencil_test_enable         = {};
        vk::StencilOpState  front                       = {};
        vk::StencilOpState  back                        = {};
        float               min_depth_bounds            = {};
        float               max_depth_bounds            = {};
    };

    struct VertexInputState {
        std::vector<vk::VertexInputBindingDescription>      bindings    = {};
        std::vector<vk::VertexInputAttributeDescription>    attributes  = {};
    };

    struct RenderPipelineStateDescription {
        ManagedShared<Function>                         vertexFunction              = {};
        ManagedShared<Function>                         fragmentFunction            = {};
        ManagedShared<TessellationState>                tessellationState           = MakeShared<TessellationState>();
        ManagedShared<MultisampleState>                 multisampleState            = MakeShared<MultisampleState>();
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
        bool                                            isAlphaToCoverageEnabled    = {};
        bool                                            isAlphaToOneEnabled         = {};
    };

    struct RenderPipelineState : ManagedObject<RenderPipelineState> {
        ManagedShared<Device>                           device                      = {};
        ManagedShared<Function>                         vertexFunction              = {};
        ManagedShared<Function>                         fragmentFunction            = {};
        ManagedShared<TessellationState>                tessellationState           = {};
        ManagedShared<MultisampleState>                 multisampleState            = {};
        VertexInputState                                vertexInputState            = {};
        uint32_t                                        viewMask                    = {};
        vk::Format                                      depthAttachmentFormat       = {};
        vk::Format                                      stencilAttachmentFormat     = {};
        RenderPipelineColorAttachmentFormatArray        colorAttachmentFormats      = {};
        RenderPipelineColorBlendAttachmentStateArray    colorBlendAttachments       = {};
        vk::PrimitiveTopology                           inputPrimitiveTopology      = {};
        bool                                            primitiveRestartEnable      = {};
        bool                                            isAlphaToCoverageEnabled    = {};
        bool                                            isAlphaToOneEnabled         = {};

        vk::PipelineCache                               cache                       = {};
        vk::PipelineLayout                              pipelineLayout              = {};
        std::map<std::size_t, vk::Pipeline>             pipelines                   = {};
        std::vector<vk::DescriptorSetLayout>            bindGroupLayouts            = {};

        explicit RenderPipelineState(ManagedShared<Device> device);
        ~RenderPipelineState() override;
    };
}