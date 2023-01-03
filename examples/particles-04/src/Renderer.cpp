#include "Renderer.hpp"
#include "Assets.hpp"
#include "Signal.hpp"

#include "glm/ext.hpp"
#include "fmt/format.h"

#include <random>

struct ShaderData {
    alignas(16) glm::mat4x4 g_proj_matrix;
    alignas(16) glm::mat4x4 g_view_matrix;
};

struct Particle {
    glm::vec3 position = {};
    glm::vec3 velocity = {};
    glm::vec4 color = {};

    float_t lifetime = 0.0F;
    float_t time = 0.0F;
};

struct ParticleSystem : gfx::Referencing {
private:
    struct Instance {
        glm::vec3 position;
        glm::vec4 color;
    };

private:
    gfx::SharedPtr<gfx::Device> mDevice = {};
    gfx::SharedPtr<gfx::Buffer> mQuadIndexBuffer = {};
    gfx::SharedPtr<gfx::Buffer> mQuadVertexBuffer = {};
    gfx::SharedPtr<gfx::Buffer> mInstanceVertexBuffer = {};
    gfx::SharedPtr<gfx::RenderPipelineState> mRenderPipelineState = {};

    size_t mInstanceCount = {};
    std::vector<Particle> mParticles = {};
    std::vector<Instance> mInstances = {};

    Signal<void(Particle&)> mParticleDeathEvent = {};
    Signal<void(Particle&, float_t)> mParticleUpdateEvent = {};

public:
    explicit ParticleSystem(gfx::SharedPtr<gfx::Device> device, size_t capacity) : mDevice(std::move(device)) {
        mParticles.resize(capacity);
        mInstances.resize(capacity);

        buildShaders();
        buildBuffers();
    }

private:
    void buildShaders() {
        auto vertexLibrary = mDevice->newLibrary(Assets::readFile("shaders/particles.vert.spv"));
        auto fragmentLibrary = mDevice->newLibrary(Assets::readFile("shaders/particles.frag.spv"));

        auto vertexFunction = vertexLibrary->newFunction("main");
        auto fragmentFunction = fragmentLibrary->newFunction("main");

        gfx::RenderPipelineStateDescription description = {};
        description.vertexDescription = gfx::RenderPipelineVertexDescription{
            .layouts = {{
                vk::VertexInputBindingDescription{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
                vk::VertexInputBindingDescription{1, sizeof(Instance), vk::VertexInputRate::eInstance}
            }},
            .attributes = {{
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
                vk::VertexInputAttributeDescription{1, 1, vk::Format::eR32G32B32Sfloat, offsetof(Instance, position)},
                vk::VertexInputAttributeDescription{2, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(Instance, color)},
            }}
        };
        description.inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList);

        description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        description.depthStencilState.setDepthTestEnable(false);
        description.attachments[0].setBlendEnable(true);
        description.attachments[0].setColorBlendOp(vk::BlendOp::eAdd);
        description.attachments[0].setAlphaBlendOp(vk::BlendOp::eAdd);
        description.attachments[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
        description.attachments[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);


        description.setVertexFunction(vertexFunction);
        description.setFragmentFunction(fragmentFunction);

        mRenderPipelineState = mDevice->newRenderPipelineState(description);
    }

    void buildBuffers() {
        std::vector<glm::vec3> quadVertices = {
            glm::vec3(-0.05F, -0.05F, 0.0F),
            glm::vec3(-0.05F, +0.05F, 0.0F),
            glm::vec3(+0.05F, +0.05F, 0.0F),
            glm::vec3(+0.05F, -0.05F, 0.0F)
        };

        std::vector<uint32_t> quadIndices = {
            0, 1, 2,
            0, 2, 3
        };

        mQuadIndexBuffer = mDevice->newBuffer(vk::BufferUsageFlagBits::eIndexBuffer, quadIndices.data(), quadIndices.size() * sizeof(uint32_t), VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
        mQuadVertexBuffer = mDevice->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, quadVertices.data(), quadVertices.size() * sizeof(glm::vec3), VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
        mInstanceVertexBuffer = mDevice->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, mInstances.size() * sizeof(Instance), VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
    }

public:
    void update(float_t dt, bool flag) {
        mInstanceCount = 0;
        for (auto& particle : mParticles) {
            if (particle.lifetime <= 0.0F) {
                continue;
            }

            particle.lifetime -= dt;
            particle.position += particle.velocity * dt;

            mParticleUpdateEvent(particle, dt);

            if (particle.lifetime <= 0.0F) {
                mParticleDeathEvent(particle);
            }

            mInstances.at(mInstanceCount).position = particle.position;
            mInstances.at(mInstanceCount).color = particle.color;
            mInstanceCount += 1;
        }

        if (mInstanceCount > 0) {
            std::memcpy(mInstanceVertexBuffer->contents(), mInstances.data(), mInstanceCount * sizeof(Instance));
            mInstanceVertexBuffer->didModifyRange(0, mInstanceCount * sizeof(Instance));
        }
    }

    auto renderPipelineState() -> gfx::SharedPtr<gfx::RenderPipelineState> {
        return mRenderPipelineState;
    }

    void draw(const gfx::SharedPtr<gfx::CommandBuffer>& cmd) {
        if (mInstanceCount == 0) {
            return;
        }

        cmd->bindIndexBuffer(mQuadIndexBuffer, 0, vk::IndexType::eUint32);
        cmd->bindVertexBuffer(0, mQuadVertexBuffer, 0);
        cmd->bindVertexBuffer(1, mInstanceVertexBuffer, 0);
        cmd->drawIndexed(6, mInstanceCount, 0, 0, 0);
    }

    void emit(const glm::vec3 &position, const glm::vec4 &color, const glm::vec3 &velocity, float_t lifetime) {
        int32_t index = -1;
        for (int32_t i = 0; i < mParticles.size(); ++i) {
            if (mParticles[i].lifetime <= 0.0F) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            return;
        }

        mParticles[index].position = position;
        mParticles[index].velocity = velocity;
        mParticles[index].color = color;
        mParticles[index].lifetime = lifetime;
        mParticles[index].time = lifetime;
    }

    auto deathEvent() -> Signal<void(Particle&)>& {
        return mParticleDeathEvent;
    }

    auto updateEvent() -> Signal<void(Particle&, float_t)>& {
        return mParticleUpdateEvent;
    }
};

struct ParticleEmitter : gfx::Referencing {
protected:
    gfx::SharedPtr<ParticleSystem> mParticleSystem;
    float_t mEmitRate = {};
    float_t mTime = {};

public:
    ParticleEmitter(gfx::SharedPtr<ParticleSystem> particleSystem, float_t emitRate)
    : mParticleSystem(std::move(particleSystem)), mEmitRate(std::max(emitRate, 0.01F)) {}

public:
    virtual void emit() = 0;

    void update(float_t dt) {
        mTime += dt;
        while (mTime >= mEmitRate) {
            mTime -= mEmitRate;
            emit();
        }
    }
};

struct RocketParticleEmitter : ParticleEmitter {
private:
    std::default_random_engine random_engine;

public:
    using ParticleEmitter::ParticleEmitter;

    void emit() override {
        glm::vec3 velocity = {};
        velocity.x = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);
        velocity.y = 2.0F;
        velocity.z = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);

        glm::vec4 color = {};
        color.r = std::uniform_real_distribution(0.0F, 1.0F)(random_engine);
        color.g = std::uniform_real_distribution(0.0F, 1.0F)(random_engine);
        color.b = std::uniform_real_distribution(0.0F, 1.0F)(random_engine);
        color.a = 1.0F;

        float_t lifetime = std::uniform_real_distribution(1.0F, 2.0F)(random_engine);

        mParticleSystem->emit({}, color, velocity * 5.0F, lifetime);
    }
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

Renderer::Renderer(gfx::SharedPtr<gfx::Device> device_) : device(std::move(device_)) {
    commandQueue = device->newCommandQueue();
    commandBuffer = commandQueue->commandBuffer();

    explosionParticleSystem = gfx::TransferPtr(new ParticleSystem(device, 1000));
    explosionParticleSystem->updateEvent().connect<&Renderer::onExplosionParticleUpdate>(this);

    sparkleParticleSystem = gfx::TransferPtr(new ParticleSystem(device, 1000));
    sparkleParticleSystem->updateEvent().connect<&Renderer::onSparkleParticleUpdate>(this);

    rocketParticleSystem = gfx::TransferPtr(new ParticleSystem(device, 1000));
    rocketParticleSystem->updateEvent().connect<&Renderer::onRocketParticleUpdate>(this);
    rocketParticleSystem->deathEvent().connect<&Renderer::onRocketParticleDeath>(this);
    rocketParticleEmitter = gfx::TransferPtr(new RocketParticleEmitter(rocketParticleSystem, 2.5F));
}

void Renderer::update(float_t dt) {
    g_proj_matrix = perspective(glm::radians(60.0F), screenSize.x / screenSize.y, 0.03F, 1000.0F);
    g_view_matrix = glm::lookAtLH(glm::vec3(25.0F, 10.0F, 0.0F), glm::vec3(0.0F, 10.0F, 0.0F), glm::vec3(0, 1, 0));

    rocketParticleEmitter->update(dt);
    rocketParticleSystem->update(dt, false);
    sparkleParticleSystem->update(dt, false);
    explosionParticleSystem->update(dt, true);
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
    rendering_viewport.setMinDepth(0.0F);
    rendering_viewport.setMaxDepth(1.0F);

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

    commandBuffer->setRenderPipelineState(rocketParticleSystem->renderPipelineState());
    commandBuffer->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);

    commandBuffer->beginRendering(rendering_info);
    commandBuffer->setScissor(0, rendering_area);
    commandBuffer->setViewport(0, rendering_viewport);

    rocketParticleSystem->draw(commandBuffer);
    sparkleParticleSystem->draw(commandBuffer);
    explosionParticleSystem->draw(commandBuffer);

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
    screenSize = glm::f32vec2(x, y);
}

void Renderer::onRocketParticleUpdate(Particle& particle, float_t dt) {
    particle.color.a = std::clamp(particle.lifetime / particle.time, 0.0F, 1.0F);

    glm::vec3 velocity = {};
    velocity.x = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine) * 0.5F;
    velocity.y = 0.0F;
    velocity.z = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine) * 0.5F;

    float_t lifetime = std::uniform_real_distribution(0.0F, 1.0F)(random_engine);

    sparkleParticleSystem->emit(particle.position, particle.color, velocity, lifetime);
}

void Renderer::onRocketParticleDeath(Particle& particle) {
    for (int i = 0; i < 250; ++i) {
        glm::vec3 velocity = {};
        velocity.x = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);
        velocity.y = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);
        velocity.z = std::uniform_real_distribution(-1.0F, 1.0F)(random_engine);

        glm::vec4 color = particle.color;
        color.a = 1.0F;

        explosionParticleSystem->emit(particle.position, color, velocity * 5.0F, 1.0F);
    }
}

void Renderer::onSparkleParticleUpdate(Particle& particle, float_t dt) {
    particle.color.a = std::clamp(particle.lifetime / particle.time, 0.0F, 1.0F);
    particle.velocity.y -= 9.8f * dt;
    particle.velocity.y = std::max(particle.velocity.y, -1.0F);
}

void Renderer::onExplosionParticleUpdate(Particle& particle, float_t dt) {
    particle.color.a = std::clamp(particle.lifetime / particle.time, 0.0F, 1.0F);
}