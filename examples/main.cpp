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

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "camera.hpp"

enum class Example {
    SDF,
    Cube
};

struct SDFGlobals {
    vk::Extent2D Resolution;
    f32 Time;
};

struct CubeGlobals {
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewProjectionMatrix;

    // todo: move to per-object data
    glm::mat4 ModelMatrix;
};

struct DrawList {
    std::vector<u32> indices = {};
    std::vector<vfx::Vertex> vertices = {};

    void clear() {
        indices.clear();
        vertices.clear();
    }

    void addQuad(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, u32 color) {
        indices.reserve(6);
        for (i32 i : {0, 1, 2, 0, 2, 3}) {
            indices.emplace_back(i + vertices.size());
        }

        vertices.reserve(4);
        vertices.emplace_back(vfx::Vertex{.position = p0, .color = color});
        vertices.emplace_back(vfx::Vertex{.position = p1, .color = color});
        vertices.emplace_back(vfx::Vertex{.position = p2, .color = color});
        vertices.emplace_back(vfx::Vertex{.position = p3, .color = color});
    }

    void drawBox(glm::vec3 const& from, glm::vec3 const& to) {
        auto const min = glm::min(from, to);
        auto const max = glm::max(from, to);

        // front
        addQuad(
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            0xFF0000FF
        );

        // back
        addQuad(
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            0xFF00FF00
        );

        // right
        addQuad(
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(max.x, max.y, min.z),
            0xFFFF0000
        );

        // left
        addQuad(
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(min.x, max.y, min.z),
            0xFF00FFFF
        );

        // up
        addQuad(
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(max.x, max.y, min.z),
            0xFFFF00FF
        );

        // down
        addQuad(
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, min.y, min.z),
            0xFFFFFFFF
        );
    }
};

struct Demo : vfx::Application, vfx::WindowDelegate {
    Arc<vfx::Window> window{};
    Arc<vfx::Context> context{};
    Arc<vfx::Swapchain> swapchain{};

    Arc<vfx::CommandQueue> commandQueue{};
    Arc<vfx::PipelineState> sdfPipelineState{};
    Arc<vfx::PipelineState> cubePipelineState{};
    Arc<vfx::PipelineState> presentPipelineState{};

    Arc<vfx::Sampler> sampler{};
    Arc<vfx::Texture> colorAttachmentTexture{};
    Arc<vfx::Texture> depthAttachmentTexture{};

    // todo: bindless
    vk::DescriptorPool descriptor_pool{};
    std::vector<vk::DescriptorSet> descriptor_sets{};

    Arc<vfx::Mesh> cube{};
    Arc<Widgets> widgets{};

    SDFGlobals sdfGlobals{};
    CubeGlobals cubeGlobals{};
    f32 timeSinceStart = 0.0f;

    Example example = Example::SDF;

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

        createSDFPipelineState();
        createCubePipelineState();
        createPresentPipelineState();
        updateAttachmentDescriptors();

        DrawList drawList{};
        drawList.drawBox(glm::vec3(-1), glm::vec3(1));

        cube = Arc<vfx::Mesh>::alloc(context);

        cube->setIndices(drawList.indices);
        cube->setVertices(drawList.vertices);
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

            timeSinceStart += dt;

            pollEvents();

            widgets->beginFrame();
            drawGui();
            widgets->endFrame();
            
            draw();
        }
    }

private:
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
        // todo: move to a better place
        {
            auto image_memory_barrier = vk::ImageMemoryBarrier{};
            image_memory_barrier.setSrcAccessMask(vk::AccessFlags{});
            image_memory_barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
            image_memory_barrier.setOldLayout(vk::ImageLayout::eUndefined);
            image_memory_barrier.setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
            image_memory_barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            image_memory_barrier.setImage(depthAttachmentTexture->image);
            image_memory_barrier.setSubresourceRange(vk::ImageSubresourceRange{
                vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
                0,
                1,
                0,
                1
            });

            cmd->handle.pipelineBarrier(
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                {},
                0, nullptr,
                0, nullptr,
                1, &image_memory_barrier
            );
        }

        auto area = vk::Rect2D{};
        area.setOffset(vk::Offset2D{0, 0});
        area.setExtent(swapchain->getDrawableSize());

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(area.extent.width));
        viewport.setHeight(f32(area.extent.height));
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);

        cmd->setScissor(0, area);
        cmd->setViewport(0, viewport);

        if (example == Example::SDF) {
            auto sdf_rendering_info = vfx::RenderingInfo{};
            sdf_rendering_info.renderArea = area;
            sdf_rendering_info.layerCount = 1;
            sdf_rendering_info.colorAttachments[0].texture = colorAttachmentTexture;
            sdf_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            sdf_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
            sdf_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
            sdf_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

            cmd->beginRendering(sdf_rendering_info);
            cmd->setPipelineState(sdfPipelineState);
            cmd->handle.pushConstants(sdfPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(SDFGlobals), &sdfGlobals);
            cmd->draw(6, 1, 0, 0);
            cmd->endRendering();
        } else if (example == Example::Cube) {
            auto cube_rendering_info = vfx::RenderingInfo{};
            cube_rendering_info.renderArea = area;
            cube_rendering_info.layerCount = 1;
            cube_rendering_info.colorAttachments[0].texture = colorAttachmentTexture;
            cube_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            cube_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
            cube_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
            cube_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

            cube_rendering_info.depthAttachment.texture = depthAttachmentTexture;
            cube_rendering_info.depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
            cube_rendering_info.depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            cube_rendering_info.depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            cube_rendering_info.depthAttachment.clearDepth = 1.0f;

            cubeGlobals.ViewMatrix = glm::lookAtLH(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
            cubeGlobals.ProjectionMatrix = Camera::getInfinityProjectionMatrix(60.0f, f32(area.extent.width) / f32(area.extent.height), 0.01f);
            cubeGlobals.ViewProjectionMatrix = cubeGlobals.ProjectionMatrix * cubeGlobals.ViewMatrix;

            cubeGlobals.ModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(timeSinceStart) * 50.0f, glm::vec3(0, 1, 0));

            cmd->beginRendering(cube_rendering_info);
            cmd->setPipelineState(cubePipelineState);
            cmd->handle.pushConstants(cubePipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(CubeGlobals), &cubeGlobals);

            cmd->handle.bindVertexBuffers(0, cube->vertexBuffer->handle, vk::DeviceSize{0});
            cmd->handle.bindIndexBuffer(cube->indexBuffer->handle, 0, vk::IndexType::eUint32);

            cmd->drawIndexed(cube->indexCount, 1, 0, 0, 0);
            cmd->endRendering();
        }

        auto gui_rendering_info = vfx::RenderingInfo{};
        gui_rendering_info.renderArea = area;
        gui_rendering_info.layerCount = 1;
        gui_rendering_info.colorAttachments[0].texture = colorAttachmentTexture;
        gui_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        gui_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
        gui_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        cmd->beginRendering(gui_rendering_info);
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

        encodePresent(cmd, drawable);

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

    void drawGui() {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::Begin("Stats");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        std::array<const char*, 2> items = { "SDF", "Cube" };

        i32 item = i32(example);
        if (ImGui::Combo(items[u64(example)], &item, items.data(), items.size())) {
            example = Example(item);
        }
        ImGui::End();
    }

    void createSDFPipelineState() {
        vfx::PipelineStateDescription description{};

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();

        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4235
        description.depthAttachmentFormat = context->depthStencilFormat;

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.rasterizationState.lineWidth = 1.0f;

        auto vertexLibrary = context->makeLibrary(Assets::read_file("shaders/sdf.vert.spv"));
        auto fragmentLibrary = context->makeLibrary(Assets::read_file("shaders/sdf.frag.spv"));

        description.vertexFunction = vertexLibrary->makeFunction("main");
        description.fragmentFunction = fragmentLibrary->makeFunction("main");

        sdfPipelineState = context->makePipelineState(description);
    }

    void createCubePipelineState() {
        vfx::PipelineStateDescription description{};

        description.bindings = {
            {0, sizeof(vfx::Vertex), vk::VertexInputRate::eVertex}
        };

        description.attributes = {
            {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vfx::Vertex, position) },
            {1, 0, vk::Format::eR8G8B8A8Unorm, offsetof(vfx::Vertex, color) }
        };

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();
        description.depthAttachmentFormat = context->depthStencilFormat;

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.depthStencilState.depthTestEnable = VK_TRUE;
        description.depthStencilState.depthWriteEnable = VK_TRUE;
        description.depthStencilState.depthCompareOp = vk::CompareOp::eLess;

        description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        description.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
        description.rasterizationState.lineWidth = 1.0f;

        auto vertexLibrary = context->makeLibrary(Assets::read_file("shaders/cube.vert.spv"));
        auto fragmentLibrary = context->makeLibrary(Assets::read_file("shaders/cube.frag.spv"));

        description.vertexFunction = vertexLibrary->makeFunction("main");
        description.fragmentFunction = fragmentLibrary->makeFunction("main");

        cubePipelineState = context->makePipelineState(description);
    }

    void createPresentPipelineState() {
        vfx::PipelineStateDescription description{};

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();

        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4235
        description.depthAttachmentFormat = context->depthStencilFormat;

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

    void encodePresent(vfx::CommandBuffer* cmd, vfx::Drawable* drawable) {
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
        cmd->handle.bindPipeline(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipeline);
        cmd->draw(6, 1, 0, 0);
        cmd->endRendering();
    }

    void setDrawableSize(const vk::Extent2D& size) {
        sdfGlobals.Resolution = size;

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
