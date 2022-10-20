#include "Renderer.hpp"
#include "ImGuiRenderer.hpp"
#include "Time.hpp"
#include "GameObject.hpp"
#include "Application.hpp"
#include "Camera.hpp"
#include "imgui.h"

Renderer::Renderer(const Arc<vfx::Context>& context, const Arc<vfx::Swapchain>& swapchain, const Arc<Window>& window) : context(context), swapchain(swapchain) {
    imgui = Arc<ImGuiRenderer>::alloc(context, window);
    commandQueue = context->makeCommandQueue();

    createSampler();
    createAttachments();
    createSDFPipelineState();
    createCubePipelineState();
    createPresentPipelineState();
    updateAttachmentDescriptors();

    gameObject = Arc<GameObject>::alloc(context);
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
    area.setExtent(swapchain->drawableSize);

    auto viewport = vk::Viewport{};
    viewport.setWidth(f32(area.extent.width));
    viewport.setHeight(f32(area.extent.height));
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    cmd->setScissor(0, area);
    cmd->setViewport(0, viewport);

    f32 aspect = viewport.width / viewport.height;

    globals.Time = Time::timeSinceStart;
    globals.CameraPosition = glm::vec3(glm::sin(globals.Time) * 3.0f, 3.0f, glm::cos(globals.Time) * 3.0f);
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
        cmd->handle->pushConstants(sdfPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);
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
        cmd->handle->pushConstants(cubePipelineState->pipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(Globals), &globals);

        gameObject->render(cmd);

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
    imgui->draw(cmd);
    cmd->endRendering();

    auto drawable = swapchain->nextDrawable();

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

    std::array<const char*, 2> items = { "SDF", "Cube" };

    i32 item = i32(example);
    if (ImGui::Combo(items[u64(example)], &item, items.data(), items.size())) {
        example = Example(item);
    }

    if (ImGui::Checkbox("VSync", &swapchain->displaySyncEnabled)) {
        context->device->waitIdle();

        swapchain->updateDrawables();
    }

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

    cmd->beginRendering(present_rendering_info);
    cmd->setViewport(0, viewport);

    vfx::int1 isDepthAttachment = 0;
//        if (example == Example::SDF) {
//            cmd->setScissor(0, area);
//            cmd->setPipelineState(presentPipelineState);
//            cmd->handle->pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(isDepthAttachment), &isDepthAttachment);
//            cmd->handle->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, 1, &descriptor_sets[0], 0, nullptr);
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
    cmd->handle->pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(isDepthAttachment), &isDepthAttachment);
    cmd->handle->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, 1, &descriptor_sets[0], 0, nullptr);
    cmd->draw(6, 1, 0, 0);

    isDepthAttachment = 1;

    cmd->setScissor(0, depthArea);
    cmd->handle->pushConstants(presentPipelineState->pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(isDepthAttachment), &isDepthAttachment);
    cmd->handle->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, presentPipelineState->pipelineLayout, 0, 1, &descriptor_sets[1], 0, nullptr);
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

void Renderer::createSampler() {
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

void Renderer::createAttachments() {
    auto size = swapchain->drawableSize;

    globals.Resolution = vfx::int2(size.width, size.height);

    auto color_texture_description = vfx::TextureDescription{
        .format = swapchain->pixelFormat,
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

void Renderer::createSDFPipelineState() {
    vfx::PipelineStateDescription description{};

    description.colorAttachmentFormats[0] = swapchain->pixelFormat;
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

    auto vertexLibrary = context->makeLibrary(Assets::readFile("shaders/sdf.vert.spv"));
    auto fragmentLibrary = context->makeLibrary(Assets::readFile("shaders/sdf.frag.spv"));

    description.vertexFunction = vertexLibrary->makeFunction("main");
    description.fragmentFunction = fragmentLibrary->makeFunction("main");

    sdfPipelineState = context->makePipelineState(description);
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

    description.colorAttachmentFormats[0] = swapchain->pixelFormat;
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

    auto vertexLibrary = context->makeLibrary(Assets::readFile("shaders/cube.vert.spv"));
    auto fragmentLibrary = context->makeLibrary(Assets::readFile("shaders/cube.frag.spv"));

    description.vertexFunction = vertexLibrary->makeFunction("main");
    description.fragmentFunction = fragmentLibrary->makeFunction("main");

    cubePipelineState = context->makePipelineState(description);
}

void Renderer::createPresentPipelineState() {
    vfx::PipelineStateDescription description{};

    description.colorAttachmentFormats[0] = swapchain->pixelFormat;

    description.attachments[0].blendEnable = false;
    description.attachments[0].colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA;

    description.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
    description.rasterizationState.lineWidth = 1.0f;

    auto vertexLibrary = context->makeLibrary(Assets::readFile("shaders/blit.vert.spv"));
    auto fragmentLibrary = context->makeLibrary(Assets::readFile("shaders/blit.frag.spv"));

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
    descriptor_pool = context->device->createDescriptorPoolUnique(pool_create_info, nullptr);

    descriptor_sets.resize(2);

    auto ds_allocate_info = vk::DescriptorSetAllocateInfo{};
    ds_allocate_info.setDescriptorPool(*descriptor_pool);
    ds_allocate_info.setSetLayouts(presentPipelineState->descriptorSetLayouts);
    descriptor_sets[0] = context->device->allocateDescriptorSets(ds_allocate_info)[0];

    ds_allocate_info.setSetLayouts(presentPipelineState->descriptorSetLayouts);
    descriptor_sets[1] = context->device->allocateDescriptorSets(ds_allocate_info)[0];
}

void Renderer::updateAttachmentDescriptors() {
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
    context->device->updateDescriptorSets({
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
