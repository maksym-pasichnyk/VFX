#pragma once

#include <pass.hpp>
#include <signal.hpp>
#include <assets.hpp>
#include <glm/glm.hpp>
#include <context.hpp>
#include <swapchain.hpp>

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

    Camera(f32 fov, f32 aspect) {
        view = glm::mat4(1.0f); // glm::inverse(camera.local_to_world_matrix());
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);

//        for (u64 i = 0; i < vfx::Context::MAX_FRAMES_IN_FLIGHT; ++i) {
//            uniforms[i] = context.create_buffer(vfx::Buffer::Target::Constant, sizeof(CameraProperties));
//        }
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

    vfx::Buffer* globals{};
    vfx::Material* material{};

    std::vector<vk::Framebuffer> framebuffers{};

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

        globals = context.create_buffer(vfx::Buffer::Target::Constant, sizeof(CameraProperties));
        auto buffer_info = vk::DescriptorBufferInfo {
            .buffer = globals->buffer,
            .offset = 0,
            .range = vk::DeviceSize(sizeof(CameraProperties))
        };
        auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = material->descriptor_sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &buffer_info
        };
        context.logical_device.updateDescriptorSets({write_descriptor_set}, {});

        framebuffers.resize(swapchain.images.size());
        for (u64 i = 0; i < swapchain.images.size(); ++i) {
            auto fb_attachments = std::array{ swapchain.views[i], depth->view };

            vk::FramebufferCreateInfo fb_create_info{};
            fb_create_info.setRenderPass(pass.handle);
            fb_create_info.setAttachments(fb_attachments);
            fb_create_info.setWidth(swapchain.surface_extent.width);
            fb_create_info.setHeight(swapchain.surface_extent.height);
            fb_create_info.setLayers(1);

            framebuffers[i] = context.logical_device.createFramebuffer(fb_create_info);
        }
    }

    ~DefaultRenderPipeline() override {
        context.destroy_buffer(globals);
        context.destroy_texture(depth);
        context.destroy_texture(color);
        context.destroy_material(material);

        for (u64 i = 0; i < framebuffers.size(); ++i) {
            context.logical_device.destroyFramebuffer(framebuffers[i]);
        }
    }

    auto create_material() -> vfx::Material* {
        vfx::MaterialDescription description{};

        description.bindings = {
            {0, sizeof(DefaultVertexFormat), vk::VertexInputRate::eVertex}
        };

        description.attributes = {
            {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(DefaultVertexFormat, position)},
            {1, 0, vk::Format::eR8G8B8A8Unorm, offsetof(DefaultVertexFormat, color)}
        };

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

        description.shaders.emplace_back(vfx::ShaderDescription{
            .bytes = Assets::read_file("shaders/default.vert.spv"),
            .entry = "main",
            .stage  = vk::ShaderStageFlagBits::eVertex,
        });
        description.shaders.emplace_back(vfx::ShaderDescription{
            .bytes = Assets::read_file("shaders/default.frag.spv"),
            .entry = "main",
            .stage  = vk::ShaderStageFlagBits::eFragment,
        });
        return context.create_material(description, pass.handle, 0);
    }

    void begin_rendering(vk::CommandBuffer cmd) {
        auto clear_values = std::array{
            vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f})),
            vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{1.0f, 0})
        };

        auto area = vk::Rect2D{};
        area.setExtent(swapchain.surface_extent);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(pass.handle);
        begin_info.setFramebuffer(framebuffers[swapchain.current_frame]);
        begin_info.setRenderArea(area);
        begin_info.setClearValues(clear_values);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(area.extent.width));
        viewport.setHeight(f32(area.extent.height));
        viewport.setMaxDepth(1.f);

        cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
        cmd.setViewport(0, viewport);
        cmd.setScissor(0, area);
    }

    void setup(vk::CommandBuffer cmd, Camera* camera) {
        CameraProperties properties{};
        properties.projection = camera->projection;
        properties.view = camera->view;

        context.update_buffer(globals, &properties, sizeof(CameraProperties), 0);

        cmd.bindPipeline(material->pipeline_bind_point, material->pipeline);
        cmd.bindDescriptorSets(
            material->pipeline_bind_point,
            material->pipeline_layout,
            0,
            material->descriptor_sets,
            {}
        );
    }
};