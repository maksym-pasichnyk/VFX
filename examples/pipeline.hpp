#pragma once

#include <assets.hpp>
#include <graph.hpp>
#include <context.hpp>
#include <glm/glm.hpp>

struct CameraProperties {
    glm::mat4 projection;
    glm::mat4 view;
};

struct RenderPipelineSettings {};

struct RenderPipeline {
    Context& context;
    RenderGraph graph{context};
    RenderPipelineSettings settings;

    std::array<Texture*, 3> color;
    std::array<Texture*, 3> depth;
    std::array<GraphicsBuffer*, 3> camera_uniforms;
    std::array<vk::Framebuffer, 3> framebuffers;

    Material* material;
    CameraProperties camera_properties;

    explicit RenderPipeline(Context& context, const RenderPipelineSettings& settings) : context(context), settings(settings) {
        for (u64 i = 0; i < Context::MAX_FRAMES_IN_FLIGHT; ++i) {
            color[i] = context.create_texture_2d(
                    context.surface_extent.width,
                    context.surface_extent.height,
                    context.surface_format.format,
                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                    vk::ImageAspectFlagBits::eColor
            );
            depth[i] = context.create_texture_2d(
                    context.surface_extent.width,
                    context.surface_extent.height,
                    context.depth_format,
                    vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment,
                    vk::ImageAspectFlagBits::eDepth
            );
            camera_uniforms[i] = context.create_buffer(GraphicsBuffer::Target::Constant, sizeof(CameraProperties));
        }

        auto pass = graph.add_pass("gbuffer");
        graph.add_present_output(pass, "color", context.surface_format.format);
        graph.set_depth_output(pass, "depth", context.depth_format);
        graph.create_render_pass();

        graph.set_clear_color("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        graph.set_clear_depth("depth", 1.0f, 0);

        material = create_gbuffer_material();

        for (u64 i = 0; i < Context::MAX_FRAMES_IN_FLIGHT; ++i) {
            auto buffer_info = vk::DescriptorBufferInfo {
                .buffer = camera_uniforms[i]->buffer,
                .offset = 0,
                .range = vk::DeviceSize(sizeof(CameraProperties))
            };
            auto write_descriptor_set = vk::WriteDescriptorSet{
                .dstSet = material->descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &buffer_info
            };
            context.logical_device.updateDescriptorSets({write_descriptor_set}, {});
        }
    }

    ~RenderPipeline() {
        for (u64 i = 0; i < Context::MAX_FRAMES_IN_FLIGHT; ++i) {
            context.destroy_texture(depth[i]);
            context.destroy_texture(color[i]);
            context.destroy_buffer(camera_uniforms[i]);
        }
        context.destroy_material(material);

        for (u64 i = 0; i < 3; ++i) {
            if (framebuffers[i]) {
                context.logical_device.destroyFramebuffer(framebuffers[i]);
            }
        }
    }

    auto create_gbuffer_material() -> Material* {
        MaterialDescription description{};

        description.dynamic_states = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        description.create_uniform_buffer_resource(0, vk::ShaderStageFlagBits::eVertex, sizeof(CameraProperties));

        description.create_binding(vk::VertexInputRate::eVertex);
        description.create_attribute(0, vk::Format::eR32G32B32Sfloat, 12);
        description.create_attribute(0, vk::Format::eR8G8B8A8Unorm,    4);

        description.create_attachment(
            true,
            vk::BlendFactor::eSrcAlpha,
            vk::BlendFactor::eOneMinusSrcAlpha,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOneMinusSrcAlpha,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        );

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.inputAssemblyState.primitiveRestartEnable = false;

        description.viewportState.viewportCount = 1;
        description.viewportState.pViewports = nullptr;
        description.viewportState.scissorCount = 1;
        description.viewportState.pScissors = nullptr;

        description.rasterizationState.depthClampEnable        = false;
        description.rasterizationState.rasterizerDiscardEnable = false;
        description.rasterizationState.polygonMode             = vk::PolygonMode::eFill;
        description.rasterizationState.cullMode                = vk::CullModeFlagBits::eNone;
        description.rasterizationState.frontFace               = vk::FrontFace::eCounterClockwise;
        description.rasterizationState.depthBiasEnable         = false;
        description.rasterizationState.depthBiasConstantFactor = 0.0f;
        description.rasterizationState.depthBiasClamp          = 0.0f;
        description.rasterizationState.depthBiasSlopeFactor    = 0.0f;
        description.rasterizationState.lineWidth               = 1.0f;

        description.depthStencilState.depthTestEnable       = false;
        description.depthStencilState.depthWriteEnable      = false;
        description.depthStencilState.depthCompareOp        = vk::CompareOp::eNever;
        description.depthStencilState.depthBoundsTestEnable = false;
        description.depthStencilState.stencilTestEnable     = false;
        description.depthStencilState.front                 = vk::StencilOpState{};
        description.depthStencilState.back                  = vk::StencilOpState{};
        description.depthStencilState.minDepthBounds        = 0.0f;
        description.depthStencilState.maxDepthBounds        = 0.0f;

        auto vert_data = Assets::read_file("shaders/default.vert.spv");
        auto frag_data = Assets::read_file("shaders/default.frag.spv");
        description.stages.emplace_back(vk::PipelineShaderStageCreateInfo{
            .stage  = vk::ShaderStageFlagBits::eVertex,
            .module = context.create_shader_module(vert_data),
            .pName = "main"
        });
        description.stages.emplace_back(vk::PipelineShaderStageCreateInfo{
            .stage  = vk::ShaderStageFlagBits::eFragment,
            .module = context.create_shader_module(frag_data),
            .pName = "main"
        });
        return context.create_material(description, graph.pass, 0);
    }

    void set_camera_properties(const CameraProperties& properties) {
        camera_properties = properties;
    }

    void begin_render_pass(vk::CommandBuffer cmd) {
        if (framebuffers[context.current_frame]) {
            context.logical_device.destroyFramebuffer(framebuffers[context.current_frame]);
        }

        auto fb_attachments = std::vector{
            context.views[context.current_frame],
            depth[context.current_frame]->view,
        };

        vk::FramebufferCreateInfo fb_create_info{};
        fb_create_info.renderPass = graph.pass;
        fb_create_info.attachmentCount = u32(fb_attachments.size());
        fb_create_info.pAttachments = fb_attachments.data();
        fb_create_info.width = context.surface_extent.width;
        fb_create_info.height = context.surface_extent.height;
        fb_create_info.layers = 1;
        framebuffers[context.current_frame] = context.logical_device.createFramebuffer(fb_create_info);

        auto area = vk::Rect2D{};
        area.setExtent(context.surface_extent);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(graph.pass);
        begin_info.setFramebuffer(framebuffers[context.current_frame]);
        begin_info.setRenderArea(area);
        begin_info.setClearValues(graph.clear_values);

        auto viewport = vk::Viewport{};
        viewport.setWidth(static_cast<f32>(area.extent.width));
        viewport.setHeight(static_cast<f32>(area.extent.height));
        viewport.setMaxDepth(1.f);

        context.update_buffer(camera_uniforms[context.current_frame], &camera_properties, sizeof(CameraProperties), 0);

        cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
        cmd.setViewport(0, viewport);
        cmd.setScissor(0, area);
    }
};