#pragma once

#include <pass.hpp>
#include <signal.hpp>
#include <assets.hpp>
#include <context.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

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

    Camera(f32 fov, f32 aspect) : fov(fov), aspect(aspect) {
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);
    }

    void set_aspect(f32 _aspect) {
        aspect = _aspect;
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);
    }

    void set_fov(f32 _fov) {
        fov = _fov;
        projection = clip * glm::infinitePerspective(glm::radians(fov), aspect, 0.1f);
    }

    auto get_view() const -> const glm::mat4& {
        return view;
    }

    auto get_projection() const -> const glm::mat4& {
        return projection;
    }

private:
    f32 fov = 0.0f;
    f32 aspect = 0.0f;

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
};

namespace vfx {
    struct RenderPipeline {
        vfx::Context &context;

        explicit RenderPipeline(vfx::Context &context) : context(context) {}
        virtual ~RenderPipeline() = default;

        virtual void render(vk::CommandBuffer cmd, std::span<Camera*> cameras) {}
    };
}

struct DefaultRenderPipeline : vfx::RenderPipeline {
    vfx::RenderPass pass{context};

    // todo: dynamic resolution
    u32 width = 1600;
    u32 height = 1200;

    vfx::Texture* color{};
    vfx::Texture* depth{};
    vk::Sampler color_sampler{};

    vfx::Buffer* globals{};
    vfx::Material* material{};

    vk::Framebuffer framebuffer{};

    explicit DefaultRenderPipeline(vfx::Context& context, vk::Format pixelFormat) : RenderPipeline(context) {
        color = context.create_texture(
            width,
            height,
            pixelFormat,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            vk::ImageAspectFlagBits::eColor
        );
        depth = context.create_texture(
            width,
            height,
            context.depth_format,
            vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment,
            vk::ImageAspectFlagBits::eDepth
        );

        color_sampler = context.logical_device.createSampler(vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
//            .maxAnisotropy = 1.0f,
//            .minLod = -1000,
//            .maxLod = 1000
        });

        std::vector<vk::SubpassDependency> dependencies{};
        std::vector<vfx::SubpassDescription> definitions{};
        std::vector<vk::AttachmentDescription> attachments{};

        attachments.emplace_back(vk::AttachmentDescription{
            .flags = {},
            .format = pixelFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal
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
            .colorAttachments = {
                vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}
            },
            .depthStencilAttachment = vk::AttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal}
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

        auto fb_attachments = std::array{ color->view, depth->view };

        vk::FramebufferCreateInfo fb_create_info{};
        fb_create_info.setRenderPass(pass.handle);
        fb_create_info.setAttachments(fb_attachments);
        fb_create_info.setWidth(width);
        fb_create_info.setHeight(height);
        fb_create_info.setLayers(1);

        framebuffer = context.logical_device.createFramebuffer(fb_create_info);
    }

    ~DefaultRenderPipeline() override {
        context.logical_device.destroySampler(color_sampler);

        context.destroy_buffer(globals);
        context.destroy_texture(depth);
        context.destroy_texture(color);
        context.destroy_material(material);

        context.logical_device.destroyFramebuffer(framebuffer);
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
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(vk::Extent2D{width, height});

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(pass.handle);
        begin_info.setFramebuffer(framebuffer);
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

    void end_rendering(vk::CommandBuffer cmd) {
        cmd.endRenderPass();
    }

    void setup(vk::CommandBuffer cmd, Camera& camera) {
        CameraProperties properties{};
        properties.projection = camera.get_projection();
        properties.view = camera.get_view();

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