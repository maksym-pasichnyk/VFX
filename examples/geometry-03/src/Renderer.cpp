#include "Renderer.hpp"

#include "Assets.hpp"

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "tiny_gltf.h"

#include <chrono>
#include <SDL_timer.h>

struct Vertex {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec4 color;
};

struct ShaderData {
    alignas(16) glm::mat4x4 g_proj_matrix;
    alignas(16) glm::mat4x4 g_view_matrix;
};

struct Primitive {
    uint32_t baseIndex;
    uint32_t baseVertex;
    uint32_t numIndices;
    uint32_t numVertices;

    explicit Primitive(uint32_t baseIndex, uint32_t baseVertex, uint32_t numIndices, uint32_t numVertices)
    : baseIndex(baseIndex), baseVertex(baseVertex), numIndices(numIndices), numVertices(numVertices) {}
};

inline auto perspective(float_t fovy, float_t aspect, float_t zNear, float_t zFar) -> glm::mat4x4 {
    float_t range = tan(fovy * 0.5F);

    float_t x = +1.0F / (range * aspect);
    float_t y = -1.0F / (range);
    float_t z = zFar / (zFar - zNear);
    float_t a = 1.0F;
    float_t b = -(zFar * zNear) / (zFar - zNear);

    return glm::mat4x4 {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, a,
        0, 0, b, 0
    };
}

glm::mat4x4 g_proj_matrix;
glm::mat4x4 g_view_matrix;
std::vector<Primitive> primitives = {};

Renderer::Renderer(gfx::SharedPtr<gfx::Device> device_) : device(std::move(device_)) {
    commandQueue = device->newCommandQueue();
    commandBuffer = commandQueue->commandBuffer();

    buildShaders();
    buildBuffers();
}

void Renderer::buildShaders() {
    auto vertexLibrary = device->newLibrary(Assets::readFile("shaders/shader.vert.spv"));
    auto fragmentLibrary = device->newLibrary(Assets::readFile("shaders/shader.frag.spv"));

    auto vertexFunction = vertexLibrary->newFunction("main");
    auto fragmentFunction = fragmentLibrary->newFunction("main");

    gfx::RenderPipelineStateDescription description = {};
    description.vertexDescription = gfx::RenderPipelineVertexDescription{
        .layouts = {{
            vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex}
        }},
        .attributes = {{
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, color)},
        }}
    };
    description.inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

    description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
    description.attachments[0].setBlendEnable(false);

    description.setVertexFunction(vertexFunction);
    description.setFragmentFunction(fragmentFunction);

    renderPipelineState = device->newRenderPipelineState(description);
}

void Renderer::buildBuffers() {
    auto bytes = Assets::readFile("models/Fox.glb");

    std::string err = {};
    std::string warn = {};
    tinygltf::Model model = {};
    tinygltf::TinyGLTF loader = {};
    
    bool loaded = loader.LoadBinaryFromMemory(&model, &err, &warn, reinterpret_cast<const unsigned char*>(bytes.data()), bytes.size());
    if (!warn.empty()) {
        fmt::print(stderr, "{}\n", warn);
    }
    if (!err.empty()) {
        fmt::print(stderr, "{}\n", err);
    }
    if (!loaded) {
        throw std::runtime_error("Failed to load gltf file!");
    }

    std::vector<Vertex> vertices = {};
    std::vector<uint32_t> indices = {};

    for (auto& mesh : model.meshes) {
        for (auto& primitive : mesh.primitives) {
            std::vector<glm::vec3> positions = {};
            std::vector<glm::vec2> texcoords = {};

            uint32_t numIndices = 0;
            uint32_t numVertices = 0;
            uint32_t baseIndex = 0;
            uint32_t baseVertex = 0;

            for (const auto& [name, attrib] : primitive.attributes) {
                auto& accessor = model.accessors[size_t(attrib)];
                auto& bufferView = model.bufferViews[size_t(accessor.bufferView)];
                auto& buffer = model.buffers[size_t(bufferView.buffer)];
                auto stride = accessor.ByteStride(bufferView);
                auto data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

                if (name == "POSITION") {
                    assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                    positions.reserve(accessor.count);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        glm::vec3 position;
                        std::memcpy(&position, data + i * size_t(stride), sizeof(glm::vec3));
                        positions.emplace_back(position);
                    }
                } else if (name == "TEXCOORD_0") {
                    assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                    texcoords.reserve(accessor.count);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        glm::vec2 texcoord;
                        std::memcpy(&texcoord, data + i * size_t(stride), sizeof(glm::vec2));
                        texcoords.emplace_back(texcoord);
                    }
                } else if (name == "JOINTS_0") {
                    assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
//                    joints.reserve(accessor.count);
//                    for (size_t i = 0; i < accessor.count; ++i) {
//                        u16 joint0;
//                        u16 joint1;
//                        u16 joint2;
//                        u16 joint3;
//                        std::memcpy(&joint0, data + i * size_t(stride) + sizeof(u16) * 0, sizeof(u16));
//                        std::memcpy(&joint1, data + i * size_t(stride) + sizeof(u16) * 1, sizeof(u16));
//                        std::memcpy(&joint2, data + i * size_t(stride) + sizeof(u16) * 2, sizeof(u16));
//                        std::memcpy(&joint3, data + i * size_t(stride) + sizeof(u16) * 3, sizeof(u16));
//                        joints.emplace_back(std::array{u32(joint0), u32(joint1), u32(joint2), u32(joint3)});
//                    }
                } else if (name == "WEIGHTS_0") {
                    assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                    assert(stride == 16);
//                    weights.reserve(accessor.count);
//                    for (size_t i = 0; i < accessor.count; ++i) {
//                        std::array<f32, 4> weight{};
//                        std::memcpy(&weight, data + i * size_t(stride), sizeof(std::array<f32, 4>));
//                        weights.emplace_back(weight);
//                    }
                }
            }

            if (primitive.indices >= 0) {
                auto& accessor = model.accessors[size_t(primitive.indices)];
                auto& bufferView = model.bufferViews[size_t(accessor.bufferView)];
                auto& buffer = model.buffers[size_t(bufferView.buffer)];
                auto stride = accessor.ByteStride(bufferView);
                auto data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;
                indices.reserve(accessor.count);
                for (size_t i = 0; i < accessor.count; ++i) {
                    uint32_t index;
                    std::memcpy(&index, data + i * size_t(stride), sizeof(uint32_t));
                    indices.emplace_back(index);
                }
                numIndices += accessor.count;
            }

            for (const auto& [position, texcoord] : ranges::views::zip(positions, texcoords)) {
                Vertex vertex = {};
                vertex.position = position * 0.01F;
                vertex.color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F);

                vertices.emplace_back(vertex);
                numVertices += 1;
            }

            primitives.emplace_back(baseIndex, baseVertex, numIndices, numVertices);
        }
    }

    vertexBuffer = device->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex), VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
}

void Renderer::update(float_t dt) {
    static float_t angle = 0.0F;

    angle += dt * 5.0F;

    g_proj_matrix = perspective(glm::radians(60.0F), screenSize.x / screenSize.y, 0.03F, 1000.0F);

    g_view_matrix = glm::lookAtLH(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0, 1, 0));
    g_view_matrix = glm::rotate(g_view_matrix, angle, glm::vec3(0, 1, 0));
}

void Renderer::draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain) {
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
    rendering_info.setRenderArea(rendering_area);
    rendering_info.setLayerCount(1);
    rendering_info.colorAttachments()[0].setTexture(drawable->texture());
    rendering_info.colorAttachments()[0].setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
    rendering_info.colorAttachments()[0].setLoadOp(vk::AttachmentLoadOp::eClear);
    rendering_info.colorAttachments()[0].setStoreOp(vk::AttachmentStoreOp::eStore);

    ShaderData shader_data = {};
    shader_data.g_proj_matrix = g_proj_matrix;
    shader_data.g_view_matrix = g_view_matrix;

    commandBuffer->begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
    commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2{}, vk::AccessFlagBits2::eColorAttachmentWrite);

    commandBuffer->setRenderPipelineState(renderPipelineState);
    commandBuffer->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);

    commandBuffer->beginRendering(rendering_info);
    commandBuffer->setScissor(0, rendering_area);
    commandBuffer->setViewport(0, rendering_viewport);

    commandBuffer->bindVertexBuffer(0, vertexBuffer, 0);
    for (auto& primitive : primitives) {
        commandBuffer->draw(primitive.numVertices, 1, primitive.baseVertex, 0);
    }
    commandBuffer->endRendering();

    commandBuffer->changeTextureLayout(drawable->texture(), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2{});
    commandBuffer->end();
    commandBuffer->submit();
    commandBuffer->present(drawable);
    commandBuffer->waitUntilCompleted();
}

void Renderer::screenResized(const vk::Extent2D& size) {
    auto x = static_cast<float_t>(size.width);
    auto y = static_cast<float_t>(size.height);
    screenSize = simd::float2{x, y};
}

