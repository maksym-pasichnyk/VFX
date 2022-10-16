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

        auto vertexLibrary = context->makeLibrary(Assets::read_file("shaders/default.vert.spv"));
        auto fragmentLibrary = context->makeLibrary(Assets::read_file("shaders/default.frag.spv"));

        description.vertexFunction = vertexLibrary->makeFunction("main");
        description.fragmentFunction = fragmentLibrary->makeFunction("main");

        pipelineState = context->makePipelineState(description);
    }

    void draw(vfx::CommandBuffer* cmd) {
        cmd->setPipelineState(pipelineState);
        cmd->handle.pushConstants(pipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
        cmd->draw(6, 1, 0, 0);
    }
};