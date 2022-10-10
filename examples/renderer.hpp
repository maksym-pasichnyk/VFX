#pragma once

#include "camera.hpp"
#include "assets.hpp"
#include "context.hpp"
#include "swapchain.hpp"

struct Globals {
    vk::Extent2D Resolution;
    f32 Time;
};

struct Renderer {
    vfx::Context &context;

    vk::Format pixel_format;
    vk::Format depth_format;

    vk::Sampler sampler{};
    vk::Extent2D drawableSize{};
    vk::Framebuffer framebuffer{};
    
    Box<vfx::Texture> color_texture{};
    Box<vfx::Texture> depth_texture{};

    Box<vfx::Material> material{};
    Box<vfx::RenderPass> render_pass{};

    Globals globals{};

    explicit Renderer(vfx::Context& context, vk::Format pixel_format)
    : context(context)
    , pixel_format(pixel_format)
    , depth_format(context.depth_format) {
        auto sampler_description = vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat
        };
        sampler = context.logical_device.createSampler(sampler_description);

        create_render_pass();
        create_material();
    }

    ~Renderer() {
        context.logical_device.destroySampler(sampler);

        if (framebuffer) {
            context.logical_device.destroyFramebuffer(framebuffer);
        }
        if (depth_texture) {
            context.freeTexture(depth_texture);
        }
        if (color_texture) {
            context.freeTexture(color_texture);
        }

        context.freeRenderPass(render_pass);
        context.freeMaterial(material);
    }

    void setDrawableSize(const vk::Extent2D& size) {
        drawableSize = size;
        globals.Resolution = drawableSize;

        if (framebuffer) {
            context.logical_device.destroyFramebuffer(framebuffer);
        }
        if (depth_texture) {
            context.freeTexture(depth_texture);
        }
        if (color_texture) {
            context.freeTexture(color_texture);
        }

        auto color_texture_description = vfx::TextureDescription{
            .format = pixel_format,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .aspect = vk::ImageAspectFlagBits::eColor
        };
        color_texture = context.makeTexture(color_texture_description);

        auto depth_texture_description = vfx::TextureDescription{
            .format = depth_format,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .aspect = vk::ImageAspectFlagBits::eDepth
        };
        depth_texture = context.makeTexture(depth_texture_description);

        auto attachments = std::array{
            color_texture->view,
            depth_texture->view
        };

        vk::FramebufferCreateInfo fb_create_info{};
        fb_create_info.setRenderPass(render_pass->handle);
        fb_create_info.setAttachments(attachments);
        fb_create_info.setWidth(size.width);
        fb_create_info.setHeight(size.height);
        fb_create_info.setLayers(1);

        framebuffer = context.logical_device.createFramebuffer(fb_create_info);
    }

    void create_render_pass() {
        vfx::RenderPassDescription pass_description{};
        pass_description.definitions = {
            vfx::SubpassDescription{
                .colorAttachments = {
                    vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}
                },
                .depthStencilAttachment = vk::AttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal}
            }
        };
        pass_description.attachments[0].format = pixel_format;
        pass_description.attachments[0].samples = vk::SampleCountFlagBits::e1;
        pass_description.attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        pass_description.attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        pass_description.attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        pass_description.attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        pass_description.attachments[0].initialLayout = vk::ImageLayout::eUndefined;
        pass_description.attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        pass_description.attachments[1].format = depth_format;
        pass_description.attachments[1].samples = vk::SampleCountFlagBits::e1;
        pass_description.attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
        pass_description.attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
        pass_description.attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        pass_description.attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        pass_description.attachments[1].initialLayout = vk::ImageLayout::eUndefined;
        pass_description.attachments[1].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        render_pass = context.makeRenderPass(pass_description);
    }

    void create_material() {
        vfx::MaterialDescription description{};

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

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

        material = context.makeMaterial(description, render_pass, 0);
    }

    void begin_rendering(vk::CommandBuffer cmd) {
        auto clear_values = std::array{
            vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f})),
            vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{1.0f, 0})
        };

        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(drawableSize);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(render_pass->handle);
        begin_info.setFramebuffer(framebuffer);
        begin_info.setRenderArea(area);
        begin_info.setClearValues(clear_values);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawableSize.width));
        viewport.setHeight(f32(drawableSize.height));
        viewport.setMaxDepth(1.f);

        cmd.beginRenderPass(begin_info, vk::SubpassContents::eInline);
        cmd.setViewport(0, viewport);
        cmd.setScissor(0, area);
    }

    void end_rendering(vk::CommandBuffer cmd) {
        cmd.endRenderPass();
    }

    void draw(vk::CommandBuffer cmd) {
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, material->pipeline);
        cmd.pushConstants(material->pipeline_layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
        cmd.draw(6, 1, 0, 0);
    }
};