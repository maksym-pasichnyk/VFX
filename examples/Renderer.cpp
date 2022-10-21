#include "Renderer.hpp"
#include "ImGuiRenderer.hpp"
#include "Time.hpp"
#include "GameObject.hpp"
#include "Application.hpp"
#include "Camera.hpp"
#include "imgui.h"
#include "DrawList.hpp"

Renderer::Renderer(const Arc<vfx::Device>& device, const Arc<vfx::Layer>& layer, const Arc<Window>& window) : device(device), layer(layer) {
    imgui = Arc<ImGuiRenderer>::alloc(device, window);
    commandQueue = device->makeCommandQueue();

    createSampler();
    createAttachments();
    createSDFPipelineState();
    createCubePipelineState();
    createPresentPipelineState();
    updateAttachmentDescriptors();

    sceneConstantsBuffer = device->makeBuffer(
        vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
        sizeof(SceneConstants),
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    );

    sdfResourceGroup->setBuffer(sceneConstantsBuffer, 0, 0);
    cubeResourceGroup->setBuffer(sceneConstantsBuffer, 0, 0);

    gameObject = Arc<GameObject>::alloc(device);
}

void Renderer::draw() {
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
    area.setExtent(layer->drawableSize);

    auto viewport = vk::Viewport{};
    viewport.setWidth(f32(area.extent.width));
    viewport.setHeight(f32(area.extent.height));
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    cmd->setScissor(0, area);
    cmd->setViewport(0, viewport);

    f32 aspect = viewport.width / viewport.height;

    sceneConstants.Time = Time::timeSinceStart;
    sceneConstants.CameraPosition = glm::vec3(glm::sin(sceneConstants.Time) * 3.0f, 3.0f, glm::cos(sceneConstants.Time) * 3.0f);
    sceneConstants.ViewMatrix = glm::lookAtLH(sceneConstants.CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    sceneConstants.ProjectionMatrix = Camera::getInfinityProjectionMatrix(60.0f, aspect, 0.01f);
    sceneConstants.ViewProjectionMatrix = sceneConstants.ProjectionMatrix * sceneConstants.ViewMatrix;
    sceneConstants.InverseViewProjectionMatrix = glm::inverse(sceneConstants.ViewProjectionMatrix);
    sceneConstantsBuffer->update(&sceneConstants, sizeof(SceneConstants), 0);

//    cmd->handle->updateBuffer(sceneConstantsBuffer->handle, 0, sizeof(SceneConstants), &sceneConstants);
//    cmd->bufferMemoryBarrier(vk::BufferMemoryBarrier2{
//        .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
//        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
//        .dstStageMask = vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader,
//        .dstAccessMask = vk::AccessFlagBits2::eUniformRead,
//        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//        .buffer = sceneConstantsBuffer->handle,
//        .offset = 0,
//        .size = sizeof(SceneConstants)
//    });
//    cmd->flushBarriers();

    modelConstants.ModelMatrix = glm::mat4(1.0f);

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

        cmd->setPipelineState(sdfPipelineState);
        cmd->setResourceGroup(sdfPipelineState, sdfResourceGroup, 0);
        cmd->beginRendering(sdf_rendering_info);
        cmd->handle->pushConstants(sdfPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(ModelConstants), &modelConstants, device->interface);
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

        cmd->setPipelineState(cubePipelineState);
        cmd->setResourceGroup(cubePipelineState, cubeResourceGroup, 0);
        cmd->beginRendering(cube_rendering_info);
        cmd->handle->pushConstants(cubePipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(ModelConstants), &modelConstants, device->interface);
        gameObject->draw(cmd);

        cmd->endRendering();
    }

    auto drawable = layer->nextDrawable();

    encodePresent(cmd, drawable);

    cmd->end();
    cmd->submit();
    cmd->present(drawable);
}

void Renderer::update() {
    imgui->beginFrame();
    drawGui();
    imgui->endFrame();
}

void Renderer::resize() {
    createAttachments();
    updateAttachmentDescriptors();
}

void Renderer::drawGui() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Stats");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    std::array<const char*, 2> items = { "Raymarhing", "Cube" };

    i32 item = i32(example);
    if (ImGui::Combo(items[u64(example)], &item, items.data(), items.size())) {
        example = Example(item);
    }

    if (ImGui::Checkbox("VSync", &layer->displaySyncEnabled)) {
        device->handle->waitIdle(device->interface);

        layer->updateDrawables();
    }

    ImGui::Checkbox("HDR", &enableHDR);
    ImGui::BeginDisabled(!enableHDR);
    ImGui::DragFloat("Exposure", &exposure, 0.1f);
    ImGui::DragFloat("Gamma", &gamma, 0.1f);
    ImGui::EndDisabled();

    ImGui::End();
}

void Renderer::encodePresent(vfx::CommandBuffer* cmd, vfx::Drawable* drawable) {
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

    cmd->setPipelineState(presentPipelineState);
    cmd->setResourceGroup(presentPipelineState, presentResourceGroup, 0);

    cmd->beginRendering(present_rendering_info);
    cmd->setViewport(0, viewport);

    struct Settings {
        vk::Bool32 isDepthAttachment;
        vk::Bool32 isHDREnabled;
        vfx::float1 exposure;
        vfx::float1 gamma;
    } settings;

    auto colorArea = vk::Rect2D{};
    colorArea.setOffset(vk::Offset2D{0, 0});
    colorArea.setExtent(vk::Extent2D{u32(size.width) / 2, size.height});

    auto depthArea = vk::Rect2D{};
    depthArea.setOffset(vk::Offset2D{i32(size.width) / 2, 0});
    depthArea.setExtent(vk::Extent2D{u32(size.width) / 2, size.height});

    settings.isDepthAttachment = VK_FALSE;
    settings.isHDREnabled      = enableHDR ? VK_TRUE : VK_FALSE;
    settings.exposure          = exposure;
    settings.gamma             = gamma;

    cmd->setScissor(0, colorArea);
    cmd->handle->pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(Settings), &settings, device->interface);
    cmd->draw(6, 1, 0, 0);

    settings.isDepthAttachment = VK_TRUE;
    settings.isHDREnabled      = enableHDR ? VK_TRUE : VK_FALSE;
    settings.exposure          = exposure;
    settings.gamma             = gamma;

    cmd->setScissor(0, depthArea);
    cmd->handle->pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(Settings), &settings, device->interface);
    cmd->draw(6, 1, 0, 0);

    cmd->endRendering();

    auto gui_rendering_info = vfx::RenderingInfo{};
    gui_rendering_info.renderArea = vk::Rect2D{.extent = drawable->texture->size};
    gui_rendering_info.layerCount = 1;
    gui_rendering_info.colorAttachments[0].texture = drawable->texture;
    gui_rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    gui_rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eLoad;
    gui_rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

    // Blend imgui on swapchain to avoid gamma correction
    cmd->beginRendering(gui_rendering_info);
    imgui->draw(cmd);
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

void Renderer::createSampler() {
    auto sampler_description = vk::SamplerCreateInfo{
        .magFilter = vk::Filter::eNearest,
        .minFilter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eNearest,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat
    };
    sampler = device->makeSampler(sampler_description);
}

void Renderer::createAttachments() {
    auto size = layer->drawableSize;

    sceneConstants.Resolution = vfx::int2(size.width, size.height);

    auto color_texture_description = vfx::TextureDescription{
        .format = vk::Format::eR32G32B32A32Sfloat,
        .width = size.width,
        .height = size.height,
        .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled
    };
    colorAttachmentTexture = device->makeTexture(color_texture_description);

    auto depth_texture_description = vfx::TextureDescription{
        .format = vk::Format::eD32Sfloat,
        .width = size.width,
        .height = size.height,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
    };
    depthAttachmentTexture = device->makeTexture(depth_texture_description);
}

void Renderer::createSDFPipelineState() {
    vfx::PipelineStateDescription description{};

    description.colorAttachmentFormats[0] = vk::Format::eR32G32B32A32Sfloat;
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

    auto vertexLibrary = device->makeLibrary(Assets::readFile("shaders/sdf.vert.spv"));
    auto fragmentLibrary = device->makeLibrary(Assets::readFile("shaders/sdf.frag.spv"));

    description.vertexFunction = vertexLibrary->makeFunction("main");
    description.fragmentFunction = fragmentLibrary->makeFunction("main");

    sdfPipelineState = device->makePipelineState(description);
    sdfResourceGroup = device->makeResourceGroup(sdfPipelineState->descriptorSetLayouts[0], {
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1}
    });
}

void Renderer::createCubePipelineState() {
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

    description.colorAttachmentFormats[0] = vk::Format::eR32G32B32A32Sfloat;
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

    auto vertexLibrary = device->makeLibrary(Assets::readFile("shaders/cube.vert.spv"));
    auto fragmentLibrary = device->makeLibrary(Assets::readFile("shaders/cube.frag.spv"));

    description.vertexFunction = vertexLibrary->makeFunction("main");
    description.fragmentFunction = fragmentLibrary->makeFunction("main");

    cubePipelineState = device->makePipelineState(description);
    cubeResourceGroup = device->makeResourceGroup(cubePipelineState->descriptorSetLayouts[0], {
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1}
    });
}

void Renderer::createPresentPipelineState() {
    vfx::PipelineStateDescription description{};

    description.colorAttachmentFormats[0] = layer->pixelFormat;

    description.attachments[0].blendEnable = false;
    description.attachments[0].colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;

    description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
    description.rasterizationState.lineWidth = 1.0f;

    auto vertexLibrary = device->makeLibrary(Assets::readFile("shaders/blit.vert.spv"));
    auto fragmentLibrary = device->makeLibrary(Assets::readFile("shaders/blit.frag.spv"));

    description.vertexFunction = vertexLibrary->makeFunction("main");
    description.fragmentFunction = fragmentLibrary->makeFunction("main");

    presentPipelineState = device->makePipelineState(description);
    presentResourceGroup = device->makeResourceGroup(presentPipelineState->descriptorSetLayouts[0], {
        vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1},
        vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 2},
    });
}

void Renderer::updateAttachmentDescriptors() {
    presentResourceGroup->setSampler(sampler, 0);
    presentResourceGroup->setTexture(colorAttachmentTexture, 1);
    presentResourceGroup->setTexture(depthAttachmentTexture, 2);
}
