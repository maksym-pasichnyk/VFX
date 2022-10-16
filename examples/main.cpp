#include <vector>

#include "queue.hpp"
#include "window.hpp"
#include "assets.hpp"
#include "texture.hpp"
#include "context.hpp"
#include "widgets.hpp"
#include "material.hpp"
#include "drawable.hpp"
#include "swapchain.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

struct Globals {
    vk::Extent2D Resolution;
    f32 Time;
};

struct Demo : vfx::Application, vfx::WindowDelegate {
    Arc<vfx::Window> window{};
    Arc<vfx::Context> context{};
    Arc<vfx::Swapchain> swapchain{};

    Arc<Widgets> widgets{};

    Arc<vfx::CommandQueue> commandQueue{};
    Arc<vfx::PipelineState> pipelineState{};
    Arc<vfx::PipelineState> presentPipelineState{};

    Arc<vfx::Sampler> sampler{};
    Arc<vfx::Texture> colorAttachmentTexture{};
    Arc<vfx::Texture> depthAttachmentTexture{};

    Globals globals{};

    // todo: bindless
    vk::DescriptorPool descriptor_pool{};
    std::vector<vk::DescriptorSet> descriptor_sets{};

    Demo() {
        window = Arc<vfx::Window>::alloc(vfx::WindowDescription{
            .title = "Demo",
            .width = 800,
            .height = 600,
            .resizable = true,
        });
        window->delegate = this;

        context = vfx::createSystemDefaultContext();

        swapchain = Arc<vfx::Swapchain>::alloc(vfx::SwapchainDescription{
            .context = context,
            .surface = window->makeSurface(context),
            .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
            .pixelFormat = vk::Format::eB8G8R8A8Unorm,
            .presentMode = vk::PresentModeKHR::eFifo
        });
        setDrawableSize(swapchain->getDrawableSize());

        widgets = Arc<Widgets>::alloc(context, window);

        commandQueue = context->makeCommandQueue(16);

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
        createPresentPipelineState();
        updateAttachmentDescriptors();
    }

    ~Demo() override {
        context->logical_device.waitIdle();
        context->logical_device.destroyDescriptorPool(descriptor_pool);
    }

    void run() {
        f64 time = glfwGetTime();
        while (!window->windowShouldClose()) {
            f64 now = glfwGetTime();
            f32 dt = f32(now - time);
            time = now;

            pollEvents();

            widgets->beginFrame();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(0, 0));
            ImGui::Begin("Stats");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
            widgets->endFrame();

            draw();
        }
    }

    void draw() {
        auto cmd = commandQueue->makeCommandBuffer();
        cmd->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        // todo: move to a better place
        {
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
        }

        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(globals.Resolution);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(globals.Resolution.width));
        viewport.setHeight(f32(globals.Resolution.height));
        viewport.setMaxDepth(1.f);

        cmd->setScissor(0, area);
        cmd->setViewport(0, viewport);

        auto renderer_rendering_info = vfx::RenderingInfo{};
        renderer_rendering_info.renderArea = area;
        renderer_rendering_info.layerCount = 1;
        renderer_rendering_info.colorAttachments[0].texture = colorAttachmentTexture;
        renderer_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        renderer_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        renderer_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        renderer_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

        cmd->beginRendering(renderer_rendering_info);
        cmd->setPipelineState(pipelineState);
        cmd->handle.pushConstants(pipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
        cmd->draw(6, 1, 0, 0);
        cmd->endRendering();

        auto widgets_rendering_info = vfx::RenderingInfo{};
        widgets_rendering_info.renderArea = area;
        widgets_rendering_info.layerCount = 1;

        widgets_rendering_info.colorAttachments[0].texture = colorAttachmentTexture;
        widgets_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        widgets_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
        widgets_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        cmd->beginRendering(widgets_rendering_info);
        widgets->draw(cmd);
        cmd->endRendering();

        // todo: move to a better place
        {
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

        auto drawable = swapchain->nextDrawable();

        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlags{});
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            image_memory_barrier.setOldLayout(vk::ImageLayout::eUndefined);
            image_memory_barrier.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(drawable->texture->image);
            image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            cmd->handle.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                {},
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        final_render_pass(cmd, drawable);

        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
            image_memory_barrier.setDstAccessMask(vk::AccessFlags{});
            image_memory_barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
            image_memory_barrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(drawable->texture->image);
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

        cmd->end();
        cmd->submit();
        cmd->present(drawable);
    }

    void final_render_pass(vfx::CommandBuffer* cmd, vfx::Drawable* drawable) {
        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(drawable->texture->size);

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(drawable->texture->size.width));
        viewport.setHeight(f32(drawable->texture->size.height));

        cmd->setScissor(0, area);
        cmd->setViewport(0, viewport);

        cmd->setPipelineState(presentPipelineState);
        cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, descriptor_sets, {});

        auto present_rendering_info = vfx::RenderingInfo{};
        present_rendering_info.renderArea = area;
        present_rendering_info.layerCount = 1;
        present_rendering_info.colorAttachments[0].texture = drawable->texture;
        present_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        present_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        present_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        present_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

        cmd->beginRendering(present_rendering_info);
        cmd->draw(6, 1, 0, 0);
        cmd->endRendering();
    }

    void createPipelineState() {
        vfx::PipelineStateDescription description{};

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();

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

    void createPresentPipelineState() {
        vfx::PipelineStateDescription description{};

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.rasterizationState.lineWidth = 1.0f;

        auto vertexLibrary = context->makeLibrary(Assets::read_file("shaders/blit.vert.spv"));
        auto fragmentLibrary = context->makeLibrary(Assets::read_file("shaders/blit.frag.spv"));

        description.vertexFunction = vertexLibrary->makeFunction("main");
        description.fragmentFunction = fragmentLibrary->makeFunction("main");

        presentPipelineState = context->makePipelineState(description);

        auto pool_sizes = std::array{
            vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1}
        };
        auto pool_create_info = vk::DescriptorPoolCreateInfo{};
        pool_create_info.setMaxSets(1);
        pool_create_info.setPoolSizes(pool_sizes);
        descriptor_pool = context->logical_device.createDescriptorPool(pool_create_info, nullptr);

        auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
        ds_allocate_info.setDescriptorPool(descriptor_pool);
        ds_allocate_info.setSetLayouts(presentPipelineState->descriptorSetLayouts);
        descriptor_sets = context->logical_device.allocateDescriptorSets(ds_allocate_info);
    }

    void updateAttachmentDescriptors() {
        const auto image_info = vk::DescriptorImageInfo{
            .sampler = sampler->handle,
            .imageView = colorAttachmentTexture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto write_descriptor_set = vk::WriteDescriptorSet{
            .dstSet = descriptor_sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .pImageInfo = &image_info
        };
        context->logical_device.updateDescriptorSets(write_descriptor_set, {});
    }

    void windowDidResize() override {
        context->logical_device.waitIdle();

        swapchain->freeDrawables();
        swapchain->makeDrawables();

        setDrawableSize(swapchain->getDrawableSize());

        updateAttachmentDescriptors();
        draw();
    }

    void setDrawableSize(const vk::Extent2D& size) {
        globals.Resolution = size;

        auto color_texture_description = vfx::TextureDescription{
            .format = swapchain->getPixelFormat(),
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
};

auto main() -> i32 {
    try {
        Demo demo{};
        demo.run();
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }
}
