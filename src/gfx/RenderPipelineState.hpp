#pragma once

#include "Device.hpp"
#include "Function.hpp"
#include "DescriptorSet.hpp"

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

    struct InputAssemblyState {
        vk::PrimitiveTopology   topology                    = vk::PrimitiveTopology::eTriangleList;
        bool                    primitive_restart_enable    = false;
    };

    struct TessellationState {
        uint32_t patch_control_points = 3;
    };

    struct RasterizationState {
        bool                    depth_clamp_enable          = false;
        bool                    discard_enable              = false;
        vk::PolygonMode         polygon_mode                = vk::PolygonMode::eFill;
        float                   line_width                  = 1.0F;
        vk::CullModeFlagBits    cull_mode                   = vk::CullModeFlagBits::eNone;
        vk::FrontFace           front_face                  = vk::FrontFace::eClockwise;
        bool                    depth_bias_enable           = false;
        float                   depth_bias_constant_factor  = 0.0F;
        float                   depth_bias_clamp            = 0.0F;
        float                   depth_bias_slope_factor     = 0.0F;
    };

    struct MultisampleState {
        vk::SampleCountFlagBits         rasterization_samples       = vk::SampleCountFlagBits::e1;
        bool                            sample_shading_enable       = false;
        float                           min_sample_shading          = 1.0F;
        vk::Optional<vk::SampleMask>    sample_mask                 = nullptr;
        bool                            alpha_to_coverage_enable    = false;
        bool                            alpha_to_one_enable         = false;
    };

    struct DepthStencilState {
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

    struct VertexInputState {
        std::vector<vk::VertexInputBindingDescription>      bindings    = {};
        std::vector<vk::VertexInputAttributeDescription>    attributes  = {};
    };

    struct RenderPipelineStateDescription {
        Function                                        vertexFunction;
        Function                                        fragmentFunction;
        InputAssemblyState                              inputAssemblyState;
        TessellationState                               tessellationState;
        RasterizationState                              rasterizationState;
        MultisampleState                                multisampleState;
        DepthStencilState                               depthStencilState;
        VertexInputState                                vertexInputState;
        uint32_t                                        viewMask                = {};
        vk::Format                                      depthAttachmentFormat   = vk::Format::eUndefined;
        vk::Format                                      stencilAttachmentFormat = vk::Format::eUndefined;
        RenderPipelineColorAttachmentFormatArray        colorAttachmentFormats  = {};
        RenderPipelineColorBlendAttachmentStateArray    colorBlendAttachments   = {};
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