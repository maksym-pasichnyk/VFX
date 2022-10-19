#include <vector>

#include "glsl.hpp"
#include "queue.hpp"
#include "camera.hpp"
#include "assets.hpp"
#include "texture.hpp"
#include "widgets.hpp"
#include "material.hpp"
#include "drawable.hpp"
#include "swapchain.hpp"
#include "Application.hpp"

#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

enum class Example {
    SDF,
    Cube
};

struct Globals {
    vfx::float4x4 ViewMatrix;
    vfx::float4x4 ProjectionMatrix;
    vfx::float4x4 ViewProjectionMatrix;
    vfx::float4x4 InverseViewProjectionMatrix;

    vfx::float3   CameraPosition;

    vfx::int2     Resolution;
    vfx::float1   Time;

    // todo: move to per-object data
    vfx::float4x4 ModelMatrix;
};

struct DrawVertex {
    glm::vec3 position = {};
    u32       color    = {};
};

struct DrawList {
    std::vector<u32> indices = {};
    std::vector<DrawVertex> vertices = {};

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
        vertices.emplace_back(DrawVertex{.position = p0, .color = color});
        vertices.emplace_back(DrawVertex{.position = p1, .color = color});
        vertices.emplace_back(DrawVertex{.position = p2, .color = color});
        vertices.emplace_back(DrawVertex{.position = p3, .color = color});
    }

    void drawBox(glm::vec3 const& from, glm::vec3 const& to, u32 color) {
        auto const min = glm::min(from, to);
        auto const max = glm::max(from, to);

        // front
        addQuad(
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            color
        );

        // back
        addQuad(
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            color
        );

        // right
        addQuad(
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(max.x, max.y, min.z),
            color
        );

        // left
        addQuad(
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(min.x, max.y, min.z),
            color
        );

        // up
        addQuad(
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, max.y, max.z),
            glm::vec3(max.x, max.y, min.z),
            color
        );

        // down
        addQuad(
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, min.y, min.z),
            color
        );
    }
};


struct Demo : Application, WindowDelegate {
private:
    Arc<Window> window{};
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

    Arc<DrawList> cubeDrawList{};
    Arc<vfx::Buffer> cubeIndexBuffer{};
    Arc<vfx::Buffer> cubeVertexBuffer{};

    Arc<Widgets> widgets{};

    Globals globals{};
    f32 timeSinceStart = 0.0f;

    Example example = Example::Cube;

public:
    Demo() {
        window = Arc<Window>::alloc(800, 600);
        window->setTitle("Demo");
        window->delegate = this;

        setenv("VFX_ENABLE_API_VALIDATION", "1", 1);
        context = vfx::createSystemDefaultContext();
        swapchain = Arc<vfx::Swapchain>::alloc(vfx::SwapchainDescription{
            .context = context,
            .surface = window->makeSurface(context),
            .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
            .pixelFormat = vk::Format::eB8G8R8A8Unorm,
            .displaySyncEnabled = true
        });
        commandQueue = context->makeCommandQueue(16);

        widgets = Arc<Widgets>::alloc(context, window);

        createSampler();
        createAttachments();
        createSDFPipelineState();
        createCubePipelineState();
        createPresentPipelineState();
        updateAttachmentDescriptors();

        cubeDrawList = Arc<DrawList>::alloc();
        cubeDrawList->drawBox(glm::vec3(-1), glm::vec3(1), 0xFF0000FF);
        cubeDrawList->drawBox(glm::vec3(-10, -2, -10), glm::vec3(10, -2, 10), 0xFF0000FF);

        cubeIndexBuffer = context->makeBuffer(vfx::BufferUsage::Index, sizeof(u32) * cubeDrawList->indices.size());
        cubeIndexBuffer->update(cubeDrawList->indices.data(), sizeof(u32) * cubeDrawList->indices.size(), 0);

        cubeVertexBuffer = context->makeBuffer(vfx::BufferUsage::Vertex, sizeof(DrawVertex) * cubeDrawList->vertices.size());
        cubeVertexBuffer->update(cubeDrawList->vertices.data(), sizeof(DrawVertex) * cubeDrawList->vertices.size(), 0);
    }

    ~Demo() override {
        context->logical_device.waitIdle();
        context->logical_device.destroyDescriptorPool(descriptor_pool);
    }

    void run() {
        f64 time = glfwGetTime();
        while (!window->shouldClose()) {
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
        cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
            .srcAccessMask = vk::AccessFlags2{},
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = colorAttachmentTexture->image,
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            }
        });
        // todo: move to a better place
        cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            .srcAccessMask = vk::AccessFlags2{},
            .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = depthAttachmentTexture->image,
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eDepth,
                .levelCount = 1,
                .layerCount = 1
            }
        });
        cmd->flushBarriers();

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

        f32 aspect = viewport.width / viewport.height;

        globals.Time = timeSinceStart;
        globals.CameraPosition = glm::vec3(glm::sin(timeSinceStart) * 3.0f, 3.0f, glm::cos(timeSinceStart) * 3.0f);
        globals.ViewMatrix = glm::lookAtLH(globals.CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        globals.ProjectionMatrix = Camera::getInfinityProjectionMatrix(60.0f, aspect, 0.01f);
        globals.ViewProjectionMatrix = globals.ProjectionMatrix * globals.ViewMatrix;
        globals.InverseViewProjectionMatrix = glm::inverse(globals.ViewProjectionMatrix);

        globals.ModelMatrix = glm::mat4(1.0f);

        if (example == Example::SDF) {
            auto sdf_rendering_info = vfx::RenderingInfo{};
            sdf_rendering_info.renderArea = area;
            sdf_rendering_info.layerCount = 1;
            sdf_rendering_info.colorAttachments[0].texture = colorAttachmentTexture;
            sdf_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            sdf_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
            sdf_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
            sdf_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

            sdf_rendering_info.depthAttachment.texture = depthAttachmentTexture;
            sdf_rendering_info.depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
            sdf_rendering_info.depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            sdf_rendering_info.depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            sdf_rendering_info.depthAttachment.clearDepth = 0.0f;

            cmd->beginRendering(sdf_rendering_info);
            cmd->setPipelineState(sdfPipelineState);
            cmd->handle.pushConstants(sdfPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
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
            cube_rendering_info.depthAttachment.clearDepth = 0.0f;

            cmd->beginRendering(cube_rendering_info);
            cmd->setPipelineState(cubePipelineState);
            cmd->handle.pushConstants(cubePipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);

            cmd->handle.bindVertexBuffers(0, cubeVertexBuffer->handle, vk::DeviceSize{0});
            cmd->handle.bindIndexBuffer(cubeIndexBuffer->handle, 0, vk::IndexType::eUint32);

            cmd->drawIndexed(cubeDrawList->indices.size(), 1, 0, 0, 0);
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

        auto drawable = swapchain->nextDrawable();

        encodePresent(cmd, drawable);

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
        description.depthAttachmentFormat = vk::Format::eD32Sfloat;

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.depthStencilState.depthTestEnable = VK_TRUE;
        description.depthStencilState.depthWriteEnable = VK_TRUE;
        description.depthStencilState.depthCompareOp = vk::CompareOp::eGreater;

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

        vfx::PipelineVertexDescription vertexDescription{};
        vertexDescription.layouts = {{
            {0, sizeof(DrawVertex), vk::VertexInputRate::eVertex}
        }};
        vertexDescription.attributes = {{
            {0, 0, vk::Format::eR32G32B32Sfloat, offsetof(DrawVertex, position) },
            {1, 0, vk::Format::eR8G8B8A8Unorm,   offsetof(DrawVertex, color) }
        }};
        description.vertexDescription = vertexDescription;

        description.colorAttachmentFormats[0] = swapchain->getPixelFormat();
        description.depthAttachmentFormat = vk::Format::eD32Sfloat;

        description.attachments[0].blendEnable = false;
        description.attachments[0].colorWriteMask =
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        description.depthStencilState.depthTestEnable = VK_TRUE;
        description.depthStencilState.depthWriteEnable = VK_TRUE;
        description.depthStencilState.depthCompareOp = vk::CompareOp::eGreater;

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
            vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 2},
            vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 2}
        };
        auto pool_create_info = vk::DescriptorPoolCreateInfo{};
        pool_create_info.setMaxSets(2);
        pool_create_info.setPoolSizes(pool_sizes);
        descriptor_pool = context->logical_device.createDescriptorPool(pool_create_info, nullptr);

        descriptor_sets.resize(2);

        auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
        ds_allocate_info.setDescriptorPool(descriptor_pool);
        ds_allocate_info.setSetLayouts(presentPipelineState->descriptorSetLayouts);
        descriptor_sets[0] = context->logical_device.allocateDescriptorSets(ds_allocate_info)[0];

        ds_allocate_info.setSetLayouts(presentPipelineState->descriptorSetLayouts);
        descriptor_sets[1] = context->logical_device.allocateDescriptorSets(ds_allocate_info)[0];
    }

    void updateAttachmentDescriptors() {
        const auto sampler_image_info = vk::DescriptorImageInfo {
            .sampler = sampler->handle
        };
        const auto color_image_info = vk::DescriptorImageInfo{
            .imageView = colorAttachmentTexture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        const auto depth_image_info = vk::DescriptorImageInfo{
            .imageView = depthAttachmentTexture->view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };
        context->logical_device.updateDescriptorSets({
            vk::WriteDescriptorSet{
                .dstSet = descriptor_sets[0],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eSampledImage,
                .pImageInfo = &color_image_info
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptor_sets[0],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eSampler,
                .pImageInfo = &sampler_image_info
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptor_sets[1],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eSampledImage,
                .pImageInfo = &depth_image_info
            },
            vk::WriteDescriptorSet{
                .dstSet = descriptor_sets[1],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eSampler,
                .pImageInfo = &sampler_image_info
            }
        }, {});
    }

    void encodePresent(vfx::CommandBuffer* cmd, vfx::Drawable* drawable) {
        // todo: move to a better place
        cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = colorAttachmentTexture->image,
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            }
        });
        // todo: move to a better place
        cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
            .srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
            .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
            .oldLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = depthAttachmentTexture->image,
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eDepth,
                .levelCount = 1,
                .layerCount = 1
            }
        });

        // todo: move to a better place
        cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
            .srcAccessMask = vk::AccessFlags2{},
            .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = drawable->texture->image,
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            }
        });
        cmd->flushBarriers();

        auto size = drawable->texture->size;
        auto area = vk::Rect2D{.extent = size};

        auto viewport = vk::Viewport{};
        viewport.setWidth(f32(size.width));
        viewport.setHeight(f32(size.height));

        auto present_rendering_info = vfx::RenderingInfo{};
        present_rendering_info.renderArea = area;
        present_rendering_info.layerCount = 1;
        present_rendering_info.colorAttachments[0].texture = drawable->texture;
        present_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        present_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        present_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        present_rendering_info.colorAttachments[0].clearColor = vfx::ClearColor{0.0f, 0.0f, 0.0f, 0.0f};

        cmd->beginRendering(present_rendering_info);
        cmd->setViewport(0, viewport);

        vfx::int1 isDepthAttachment = 0;
//        if (example == Example::SDF) {
//            cmd->setScissor(0, area);
//            cmd->setPipelineState(presentPipelineState);
//            cmd->handle.pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(isDepthAttachment), &isDepthAttachment);
//            cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, 1, &descriptor_sets[0], 0, nullptr);
//            cmd->draw(6, 1, 0, 0);
//        } else if (example == Example::Cube) {
            auto colorArea = vk::Rect2D{};
            colorArea.setOffset(vk::Offset2D{0, 0});
            colorArea.setExtent(vk::Extent2D{u32(size.width) / 2, size.height});

            auto depthArea = vk::Rect2D{};
            depthArea.setOffset(vk::Offset2D{i32(size.width) / 2, 0});
            depthArea.setExtent(vk::Extent2D{u32(size.width) / 2, size.height});

            isDepthAttachment = 0;

            cmd->setScissor(0, colorArea);
            cmd->setPipelineState(presentPipelineState);
            cmd->handle.pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(isDepthAttachment), &isDepthAttachment);
            cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, 1, &descriptor_sets[0], 0, nullptr);
            cmd->draw(6, 1, 0, 0);

            isDepthAttachment = 1;

            cmd->setScissor(0, depthArea);
            cmd->handle.pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(isDepthAttachment), &isDepthAttachment);
            cmd->handle.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, 1, &descriptor_sets[1], 0, nullptr);
            cmd->draw(6, 1, 0, 0);
//        }

        cmd->endRendering();

        // todo: move to a better place
        cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
            .dstAccessMask = vk::AccessFlags2{},
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::ePresentSrcKHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = drawable->texture->image,
            .subresourceRange = vk::ImageSubresourceRange{
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .levelCount = 1,
                .layerCount = 1
            }
        });
        cmd->flushBarriers();
    }

    void createSampler() {
        auto sampler_description = vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eNearest,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat
        };
        sampler = context->makeSampler(sampler_description);
    }

    void createAttachments() {
        auto size = swapchain->getDrawableSize();

        globals.Resolution = {size.width, size.height};

        auto color_texture_description = vfx::TextureDescription{
            .format = swapchain->getPixelFormat(),
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled
        };
        colorAttachmentTexture = context->makeTexture(color_texture_description);

        auto depth_texture_description = vfx::TextureDescription{
            .format = vk::Format::eD32Sfloat,
            .width = size.width,
            .height = size.height,
            .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
            .components = {
                .r = vk::ComponentSwizzle::eB,
                .g = vk::ComponentSwizzle::eR,
                .b = vk::ComponentSwizzle::eR,
                .a = vk::ComponentSwizzle::eIdentity,
            }
        };
        depthAttachmentTexture = context->makeTexture(depth_texture_description);
    }

public:
    void windowDidResize() override {
        context->logical_device.waitIdle();

        swapchain->freeDrawables();
        swapchain->makeDrawables();

        createAttachments();

        updateAttachmentDescriptors();
        draw();
    }

    void windowShouldClose() override {}
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
