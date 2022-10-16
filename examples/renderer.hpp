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

    vk::Format pixelFormat;
    vk::Extent2D drawableSize{};

    Arc<vfx::Sampler> sampler{};
    Arc<vfx::Texture> colorAttachmentTexture{};
    Arc<vfx::Texture> depthAttachmentTexture{};

    Arc<vfx::PipelineState> pipelineState{};

    Globals globals{};

    explicit Renderer(const Arc<vfx::Context>& context, vk::Format pixel_format)
    : context(context)
    , pixelFormat(pixel_format) {
        auto sampler_description = vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat
        };
        sampler = context->makeSampler(sampler_description);

        createPipelineState();
    }

    ~Renderer() {
    }

    void setDrawableSize(const vk::Extent2D& size) {
        drawableSize = size;
        globals.Resolution = drawableSize;

        auto color_texture_description = vfx::TextureDescription{
            .format = pixelFormat,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .aspect = vk::ImageAspectFlagBits::eColor
        };
        colorAttachmentTexture = context->makeTexture(color_texture_description);

        auto depth_texture_description = vfx::TextureDescription{
            .format = context->depthStencilFormat,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .aspect = vk::ImageAspectFlagBits::eDepth
        };
        depthAttachmentTexture = context->makeTexture(depth_texture_description);
    }

//    void createRenderPass() {
//        vfx::RenderPassDescription description{};
//        description.definitions = {
//            vfx::SubpassDescription{
//                .colorAttachments = {
//                    vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}
//                },
//                .depthStencilAttachment = vk::AttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal}
//            }
//        };
//        description.attachments[0].format = pixelFormat;
//        description.attachments[0].samples = vk::SampleCountFlagBits::e1;
//        description.attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
//        description.attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
//        description.attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
//        description.attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
//        description.attachments[0].initialLayout = vk::ImageLayout::eUndefined;
//        description.attachments[0].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
//
//        description.attachments[1].format = depthStencilFormat;
//        description.attachments[1].samples = vk::SampleCountFlagBits::e1;
//        description.attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
//        description.attachments[1].storeOp = vk::AttachmentStoreOp::eStore;
//        description.attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
//        description.attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
//        description.attachments[1].initialLayout = vk::ImageLayout::eUndefined;
//        description.attachments[1].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
//
//        renderPass = context->makeRenderPass(description);
//    }

    void createPipelineState() {
        vfx::PipelineStateDescription description{};

        description.colorAttachmentFormats[0] = pixelFormat;
//        description.rendering->depthAttachmentFormat = depthStencilFormat;
//        description.rendering->stencilAttachmentFormat = depthStencilFormat;

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

        description.rasterizationState.lineWidth = 1.0f;

        description.vertexFunction = context->makeFunction(Assets::read_file("shaders/default.vert.spv"), "main");
        description.fragmentFunction = context->makeFunction(Assets::read_file("shaders/default.frag.spv"), "main");

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

//        auto begin_info = vk::RenderPassBeginInfo{};
//        begin_info.setRenderPass(renderPass->handle);
//        begin_info.setFramebuffer(framebuffer);
//        begin_info.setRenderArea(area);
//        begin_info.setClearValues(clear_values);
//        cmd->beginRenderPass(begin_info, vk::SubpassContents::eInline);

        auto info = vfx::RenderingInfo{};
        info.renderArea = area;
        info.layerCount = 1;

        info.colorAttachments[0].texture = colorAttachmentTexture;
        info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        info.colorAttachments[0].clearValue = clear_values[0];

//        info.depthAttachment.texture = depthAttachmentTexture;
//        info.depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
//        info.depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
//        info.depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
//        info.depthAttachment.clearValue = clear_values[1];

        auto image_memory_barrier = vk::ImageMemoryBarrier{};
        image_memory_barrier.setSrcAccessMask(vk::AccessFlags{});
        image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
        image_memory_barrier.setOldLayout(vk::ImageLayout::eUndefined);
        image_memory_barrier.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
        image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        image_memory_barrier.setImage(colorAttachmentTexture->image);
        image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        cmd->handle.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            {},
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier
        );

        cmd->beginRendering(info);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawableSize.width));
        viewport.setHeight(f32(drawableSize.height));
        viewport.setMaxDepth(1.f);

        cmd->setViewport(0, viewport);
        cmd->setScissor(0, area);
    }

    void endRendering(vfx::CommandBuffer* cmd) {
        cmd->endRendering();
//        cmd->endRenderPass();

        auto image_memory_barrier = vk::ImageMemoryBarrier{};
        image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
        image_memory_barrier.setDstAccessMask(vk::AccessFlags{});
        image_memory_barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
        image_memory_barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
        image_memory_barrier.setImage(colorAttachmentTexture->image);
        image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        cmd->handle.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            {},
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier
        );
    }

    void draw(vfx::CommandBuffer* cmd) {
        cmd->setPipelineState(pipelineState);
        cmd->handle.pushConstants(pipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
        cmd->draw(6, 1, 0, 0);
    }
};