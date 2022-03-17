#pragma once

#include <pass.hpp>
#include <signal.hpp>
#include <assets.hpp>
#include <context.hpp>
#include <glm/glm.hpp>

struct CameraProperties {
    glm::mat4 projection;
    glm::mat4 view;
};

struct Camera {
    static constexpr auto clip = glm::mat4{
        1.0f,  0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f,  0.0f, 1.0f, 0.0f,
        0.0f,  0.0f, 0.0f, 1.0f
    };

    glm::mat4 view{};
    glm::mat4 projection{};

    vfx::Context& context;
    std::array<vfx::Buffer*, 3> uniforms{};

    Camera(vfx::Context& context, f32 fov, f32 aspect) : context(context) {
        view = glm::mat4(1.0f); // glm::inverse(camera.local_to_world_matrix());
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);

        for (u64 i = 0; i < vfx::Context::MAX_FRAMES_IN_FLIGHT; ++i) {
            uniforms[i] = context.create_buffer(vfx::Buffer::Target::Constant, sizeof(CameraProperties));
        }
    }

    ~Camera() {
        for (u64 i = 0; i < vfx::Context::MAX_FRAMES_IN_FLIGHT; ++i) {
            context.destroy_buffer(uniforms[i]);
        }
    }
};

namespace vfx {
    struct RenderPipeline {
        vfx::Context &context;
        vfx::Swapchain &swapchain;

        RenderPipeline(vfx::Context &context, vfx::Swapchain &swapchain) : context(context), swapchain(swapchain) {}
        virtual ~RenderPipeline() = default;

        virtual void render(vk::CommandBuffer cmd, std::span<Camera*> cameras) {}
    };
}

struct DefaultRenderPipeline : vfx::RenderPipeline {
    vfx::RenderPass pass{context};

    vfx::Texture* color{};
    vfx::Texture* depth{};

    vfx::Material* material{};
    std::array<vk::Framebuffer, 3> framebuffers{};

    explicit DefaultRenderPipeline(vfx::Context& context, vfx::Swapchain& swapchain) : RenderPipeline(context, swapchain) {
        color = context.create_texture(
            swapchain.surface_extent.width,
            swapchain.surface_extent.height,
            swapchain.surface_format.format,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
            vk::ImageAspectFlagBits::eColor
        );
        depth = context.create_texture(
            swapchain.surface_extent.width,
            swapchain.surface_extent.height,
            context.depth_format,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment,
            vk::ImageAspectFlagBits::eDepth
        );

        std::vector<vk::SubpassDependency> dependencies{};
        std::vector<vfx::SubpassDescription> definitions{};
        std::vector<vk::AttachmentDescription> attachments{};

        attachments.emplace_back(vk::AttachmentDescription{
            .flags = {},
            .format = swapchain.surface_format.format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
        });
        attachments.emplace_back(vk::AttachmentDescription{
            .flags = {},
            .format = context.depth_format,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        });
        definitions.emplace_back(vfx::SubpassDescription{
            .pColorAttachments = {
                vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}
            },
            .pDepthStencilAttachment = vk::AttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal}
        });
        pass.init(attachments, definitions, dependencies);

        material = create_material();
    }

    ~DefaultRenderPipeline() {
        context.destroy_texture(depth);
        context.destroy_texture(color);
        context.destroy_material(material);

        for (u64 i = 0; i < 3; ++i) {
            if (framebuffers[i]) {
                context.logical_device.destroyFramebuffer(framebuffers[i]);
            }
        }
    }

    auto create_material() -> vfx::Material* {
        vfx::Material::Description description{};

        description.dynamic_states = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        description.create_uniform_resource(0, vk::ShaderStageFlagBits::eVertex, sizeof(CameraProperties));

        description.create_binding(vk::VertexInputRate::eVertex);
        description.create_attribute(0, vk::Format::eR32G32B32Sfloat, 12);
        description.create_attribute(0, vk::Format::eR8G8B8A8Unorm,    4);

        description.create_attachment(vk::PipelineColorBlendAttachmentState{
            .blendEnable = true,
            .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
            .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                              vk::ColorComponentFlagBits::eG |
                              vk::ColorComponentFlagBits::eB |
                              vk::ColorComponentFlagBits::eA
        });

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
        return context.create_material(description, pass.handle, 0);
    }

    void begin_frame(vk::CommandBuffer cmd) {
        auto attachments = std::array{
            swapchain.views[swapchain.current_frame],
            depth->view,
        };

        auto clear_values = std::array{
            vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f})),
            vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{1.0f, 0})
        };

        vk::FramebufferCreateInfo fb_create_info{};
        fb_create_info.renderPass = pass.handle;
        fb_create_info.attachmentCount = u32(attachments.size());
        fb_create_info.pAttachments = attachments.data();
        fb_create_info.width = swapchain.surface_extent.width;
        fb_create_info.height = swapchain.surface_extent.height;
        fb_create_info.layers = 1;

        if (framebuffers[swapchain.current_frame]) {
            context.logical_device.destroyFramebuffer(framebuffers[swapchain.current_frame]);
        }
        framebuffers[swapchain.current_frame] = context.logical_device.createFramebuffer(fb_create_info);

        auto area = vk::Rect2D{};
        area.setExtent(swapchain.surface_extent);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(pass.handle);
        begin_info.setFramebuffer(framebuffers[swapchain.current_frame]);
        begin_info.setRenderArea(area);
        begin_info.setClearValues(clear_values);

        auto viewport = vk::Viewport{};
        viewport.setWidth(static_cast<f32>(area.extent.width));
        viewport.setHeight(static_cast<f32>(area.extent.height));
        viewport.setMaxDepth(1.f);

        cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
        cmd.setViewport(0, viewport);
        cmd.setScissor(0, area);
    }

    void setup(vk::CommandBuffer cmd, Camera* camera) {
        auto buffer_info = vk::DescriptorBufferInfo {
            .buffer = camera->uniforms[swapchain.current_frame]->buffer,
            .offset = 0,
            .range = vk::DeviceSize(sizeof(CameraProperties))
        };
        auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = material->descriptor_sets[swapchain.current_frame],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &buffer_info
        };
        context.logical_device.updateDescriptorSets({write_descriptor_set}, {});

        CameraProperties properties{};
        properties.projection = camera->projection;
        properties.view = camera->view;

        context.update_buffer(camera->uniforms[swapchain.current_frame], &properties, sizeof(CameraProperties), 0);

        cmd.bindPipeline(material->pipeline_bind_point, material->pipeline);
        cmd.bindDescriptorSets(
            material->pipeline_bind_point,
            material->pipeline_layout,
            0,
            material->descriptor_sets[swapchain.current_frame],
            {}
        );
    }
};