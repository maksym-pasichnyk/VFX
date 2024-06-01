#include "GltfBundle.hpp"
#include "Application.hpp"

struct Game : Application {
public:
    Game() : Application("Geometry-03") {
        buildShaders();
        buildBuffers();
    }

private:
    void buildShaders() {
        auto vertexLibrary = device->newLibrary(Assets::readFile("shaders/geometry.vert.spv"));
        auto fragmentLibrary = device->newLibrary(Assets::readFile("shaders/geometry.frag.spv"));

        gfx::DepthStencilStateDescription depthStencilStateDescription;
        depthStencilState = device->newDepthStencilState(depthStencilStateDescription);

        gfx::RenderPipelineStateDescription renderPipelineStateDescription;
        renderPipelineStateDescription.vertexFunction = vertexLibrary->newFunction("main");
        renderPipelineStateDescription.fragmentFunction = fragmentLibrary->newFunction("main");
        renderPipelineStateDescription.vertexInputState = {
            .bindings = {
                vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex}
            },
            .attributes = {
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
                vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
                vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv)},
            }
        };

        renderPipelineStateDescription.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        renderPipelineStateDescription.colorBlendAttachments[0].setBlendEnable(false);

        render_pipeline_state = device->newRenderPipelineState(renderPipelineStateDescription);
        sampler = device->newSampler(vk::SamplerCreateInfo{
            .magFilter = vk::Filter::eNearest,
            .minFilter = vk::Filter::eNearest,
            .mipmapMode = vk::SamplerMipmapMode::eLinear,
            .addressModeU = vk::SamplerAddressMode::eRepeat,
            .addressModeV = vk::SamplerAddressMode::eRepeat,
            .addressModeW = vk::SamplerAddressMode::eRepeat,
        });

        texture = device->newTexture(gfx::TextureDescription{
            .width = 1,
            .height = 1,
            .format = vk::Format::eR8G8B8A8Unorm,
            .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        });

        std::array<uint8_t, 4> orange = {255, 127, 0, 255};
        texture->replaceRegion(orange.data(), sizeof(orange));
    }

    void buildBuffers() {
        gltf_bundle = GltfBundle::open("models/Fox.glb");
        for (auto& mesh : gltf_bundle.meshes) {
            mesh->uploadMeshData(device);
        }
    }

public:
    void update(float_t dt) override {
        camera_projection_matrix = getPerspectiveProjection(glm::radians(60.0F), platform->getAspectRatio(), 0.03F, 1000.0F);
        world_to_camera_matrix = glm::lookAtLH(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0, 1, 0));
        world_to_camera_matrix = glm::rotate(world_to_camera_matrix, angle, glm::vec3(0, 1, 0));
        angle += dt * 5.0F;
    }

    void render() override {
        auto drawable = swapchain->nextDrawable();
        auto drawableSize = swapchain->drawableSize();

        vk::Rect2D rendering_area = {};
        rendering_area.setOffset(vk::Offset2D{0, 0});
        rendering_area.setExtent(drawableSize);

        vk::Viewport rendering_viewport = {};
        rendering_viewport.setWidth(static_cast<float_t>(drawableSize.width));
        rendering_viewport.setHeight(static_cast<float_t>(drawableSize.height));
        rendering_viewport.setMinDepth(0.0f);
        rendering_viewport.setMaxDepth(1.0f);

        gfx::RenderingInfo rendering_info = {};
        rendering_info.renderArea = rendering_area;
        rendering_info.layerCount = 1;
        rendering_info.colorAttachments[0].texture = drawable.texture;
        rendering_info.colorAttachments[0].imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        rendering_info.colorAttachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        rendering_info.colorAttachments[0].storeOp = vk::AttachmentStoreOp::eStore;

        ShaderData shader_data = {};
        shader_data.g_proj_matrix = camera_projection_matrix;
        shader_data.g_view_matrix = world_to_camera_matrix;

        commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        auto descriptorSet = commandBuffer->newDescriptorSet(render_pipeline_state, 0);

        vk::DescriptorImageInfo sampler_info = {};
        sampler_info.setSampler(sampler->handle);

        vk::DescriptorImageInfo texture_info = {};
        texture_info.setImageView(texture->image_view);
        texture_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

        vk::WriteDescriptorSet writes[2] = {};
        writes[0].setDstSet(descriptorSet);
        writes[0].setDstBinding(0);
        writes[0].setDstArrayElement(0);
        writes[0].setDescriptorType(vk::DescriptorType::eSampler);
        writes[0].setDescriptorCount(1);
        writes[0].setPImageInfo(&sampler_info);

        writes[1].setDstSet(descriptorSet);
        writes[1].setDstBinding(1);
        writes[1].setDstArrayElement(0);
        writes[1].setDescriptorType(vk::DescriptorType::eSampledImage);
        writes[1].setDescriptorCount(1);
        writes[1].setPImageInfo(&texture_info);

        device->handle.updateDescriptorSets(2, writes, 0, nullptr, device->dispatcher);

        commandBuffer->setImageLayout(drawable.texture, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

        auto encoder = commandBuffer->newRenderCommandEncoder(rendering_info);
        encoder->setDepthStencilState(depthStencilState);
        encoder->setRenderPipelineState(render_pipeline_state);
        encoder->bindDescriptorSet(descriptorSet, 0);
        encoder->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);
        encoder->setScissor(0, rendering_area);
        encoder->setViewport(0, rendering_viewport);
        gltf_bundle.meshes.front()->draw(encoder);
        encoder->endEncoding();

        commandBuffer->setImageLayout(drawable.texture, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
        commandBuffer->end();
        commandBuffer->submit();
        commandBuffer->present(drawable);
        commandBuffer->waitUntilCompleted();
    }

private:
    float_t angle = 0.0F;
    glm::mat4x4 camera_projection_matrix = {};
    glm::mat4x4 world_to_camera_matrix = {};

    GltfBundle  gltf_bundle;

    rc<gfx::Sampler>             sampler;
    rc<gfx::Texture>             texture;
    rc<gfx::DepthStencilState>   depthStencilState;
    rc<gfx::RenderPipelineState> render_pipeline_state;
};

auto main(int argc, char** argv) -> int32_t {
    setenv("GFX_ENABLE_API_VALIDATION", "1", 1);

    Game app{};
    app.run();

    return 0;
}