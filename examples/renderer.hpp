#pragma once

#include "camera.hpp"
#include "assets.hpp"
#include "texture.hpp"
#include "context.hpp"
#include "material.hpp"
#include "swapchain.hpp"

struct Globals {
    vk::Extent2D Resolution;
    f32 Time;
};

struct Renderer {
    Arc<vfx::Context> context;

    vk::Format pixel_format;
    vk::Format depth_format;

    vk::Sampler sampler{};
    vk::Extent2D drawableSize{};
    vk::Framebuffer framebuffer{};

    Arc<vfx::Texture> colorAttachmentTexture{};
    Arc<vfx::Texture> depthAttachmentTexture{};

    Arc<vfx::RenderPass> renderPass{};
    Arc<vfx::PipelineState> pipelineState{};

    Globals globals{};

    explicit Renderer(const Arc<vfx::Context>& context, vk::Format pixel_format)
    : context(context)
    , pixel_format(pixel_format)
    , depth_format(context->depth_format) {
        auto sampler_description = vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat
        };
        sampler = context->logical_device.createSampler(sampler_description);

        createRenderPass();
        createPipelineState();
    }

    ~Renderer() {
        context->logical_device.destroySampler(sampler);

        if (framebuffer) {
            context->logical_device.destroyFramebuffer(framebuffer);
        }
        if (depthAttachmentTexture) {
            context->freeTexture(depthAttachmentTexture);
        }
        if (colorAttachmentTexture) {
            context->freeTexture(colorAttachmentTexture);
        }

        context->freePipelineState(pipelineState);
        context->freeRenderPass(renderPass);
    }

    void setDrawableSize(const vk::Extent2D& size) {
        drawableSize = size;
        globals.Resolution = drawableSize;

        if (framebuffer) {
            context->logical_device.destroyFramebuffer(framebuffer);
        }
        if (depthAttachmentTexture) {
            context->freeTexture(depthAttachmentTexture);
        }
        if (colorAttachmentTexture) {
            context->freeTexture(colorAttachmentTexture);
        }

        auto color_texture_description = vfx::TextureDescription{
            .format = pixel_format,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .aspect = vk::ImageAspectFlagBits::eColor
        };
        colorAttachmentTexture = context->makeTexture(color_texture_description);

        auto depth_texture_description = vfx::TextureDescription{
            .format = depth_format,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .aspect = vk::ImageAspectFlagBits::eDepth
        };
        depthAttachmentTexture = context->makeTexture(depth_texture_description);

        auto attachments = std::array{
            colorAttachmentTexture->view,
            depthAttachmentTexture->view
        };

        vk::FramebufferCreateInfo fb_create_info{};
        fb_create_info.setRenderPass(renderPass->handle);
        fb_create_info.setAttachments(attachments);
        fb_create_info.setWidth(size.width);
        fb_create_info.setHeight(size.height);
        fb_create_info.setLayers(1);

        framebuffer = context->logical_device.createFramebuffer(fb_create_info);
    }

    void createRenderPass() {
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

        renderPass = context->makeRenderPass(pass_description);
    }

    void createPipelineState() {
        vfx::PipelineStateDescription description{};

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

        description.rasterizationState.lineWidth = 1.0f;

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
        pipelineState = context->makePipelineState(description);
    }

    void beginRendering(vfx::CommandBuffer* cmd) {
        auto clear_values = std::array{
            vk::ClearValue{}.setColor(vk::ClearColorValue{}.setFloat32({0.0f, 0.0f, 0.0f, 0.0f})),
            vk::ClearValue{}.setDepthStencil(vk::ClearDepthStencilValue{1.0f, 0})
        };

        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(drawableSize);

        auto begin_info = vk::RenderPassBeginInfo{};
        begin_info.setRenderPass(renderPass->handle);
        begin_info.setFramebuffer(framebuffer);
        begin_info.setRenderArea(area);
        begin_info.setClearValues(clear_values);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawableSize.width));
        viewport.setHeight(f32(drawableSize.height));
        viewport.setMaxDepth(1.f);

        cmd->beginRenderPass(begin_info, vk::SubpassContents::eInline);
        cmd->setViewport(0, viewport);
        cmd->setScissor(0, area);
    }

    void endRendering(vfx::CommandBuffer* cmd) {
        cmd->endRenderPass();
    }

    void draw(vfx::CommandBuffer* cmd) {
        cmd->setPipelineState(pipelineState);
        cmd->handle.pushConstants(pipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
        cmd->draw(6, 1, 0, 0);
    }
};