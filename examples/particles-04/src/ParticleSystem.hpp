#pragma once

#include "Object.hpp"
#include "Assets.hpp"
#include "Signal.hpp"
#include "Particle.hpp"
#include "Graphics.hpp"

struct ParticleSystem : public ManagedObject {
private:
    struct Instance {
        glm::vec3 position;
        glm::vec4 color;
    };

private:
    rc<gfx::Device> mDevice;
    rc<gfx::Buffer> mQuadIndexBuffer;
    rc<gfx::Buffer> mQuadVertexBuffer;
    rc<gfx::Buffer> mInstanceVertexBuffer;
    rc<gfx::DepthStencilState> mDepthStencilState;
    rc<gfx::RenderPipelineState> mRenderPipelineState;

    size_t mInstanceCount = {};
    std::vector<Particle> mParticles = {};
    std::vector<Instance> mInstances = {};

    Signal<void(Particle&)> mParticleDeathEvent = {};
    Signal<void(Particle&, float)> mParticleUpdateEvent = {};

public:
    explicit ParticleSystem(rc<gfx::Device> device, size_t capacity) : mDevice(std::move(device)) {
        mParticles.resize(capacity);
        mInstances.resize(capacity);

        buildShaders();
        buildBuffers();
    }

private:
    void buildShaders() {
        gfx::DepthStencilStateDescription depthStencilStateDescription;
        depthStencilStateDescription.isDepthTestEnabled = true;

        mDepthStencilState = mDevice->newDepthStencilState(depthStencilStateDescription);

        auto vertexLibrary = mDevice->newLibrary(Assets::readFile("shaders/particles.vert.spv"));
        auto fragmentLibrary = mDevice->newLibrary(Assets::readFile("shaders/particles.frag.spv"));

        auto vertexInputState = rc<gfx::VertexInputState>::init();
        vertexInputState->bindings = {
            vk::VertexInputBindingDescription{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
            vk::VertexInputBindingDescription{1, sizeof(Instance), vk::VertexInputRate::eInstance}
        };
        vertexInputState->attributes = {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
            vk::VertexInputAttributeDescription{1, 1, vk::Format::eR32G32B32Sfloat, offsetof(Instance, position)},
            vk::VertexInputAttributeDescription{2, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(Instance, color)},
        };

        auto renderPipelineStateDescription = gfx::RenderPipelineStateDescription::init();
        renderPipelineStateDescription->setVertexFunction(vertexLibrary->newFunction("main"));
        renderPipelineStateDescription->setFragmentFunction(fragmentLibrary->newFunction("main"));
        renderPipelineStateDescription->setVertexInputState(vertexInputState);

        renderPipelineStateDescription->colorAttachmentFormats()[0] = vk::Format::eB8G8R8A8Unorm;
//        description.depthStencilState->depth_test_enable = true;

        renderPipelineStateDescription->colorBlendAttachments()[0].setBlendEnable(true);
        renderPipelineStateDescription->colorBlendAttachments()[0].setColorBlendOp(vk::BlendOp::eAdd);
        renderPipelineStateDescription->colorBlendAttachments()[0].setAlphaBlendOp(vk::BlendOp::eAdd);
        renderPipelineStateDescription->colorBlendAttachments()[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
        renderPipelineStateDescription->colorBlendAttachments()[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);

        mRenderPipelineState = mDevice->newRenderPipelineState(renderPipelineStateDescription);
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

        mQuadIndexBuffer = mDevice->newBuffer(vk::BufferUsageFlagBits::eIndexBuffer, quadIndices.data(), quadIndices.size() * sizeof(uint32_t), gfx::StorageMode::eShared);
        mQuadVertexBuffer = mDevice->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, quadVertices.data(), quadVertices.size() * sizeof(glm::vec3), gfx::StorageMode::eShared);
        mInstanceVertexBuffer = mDevice->newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, mInstances.size() * sizeof(Instance), gfx::StorageMode::eShared);
    }

public:
    void update(float dt, bool flag) {
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

    auto getRenderPipelineState() -> rc<gfx::RenderPipelineState> {
        return mRenderPipelineState;
    }

    void draw(const rc<gfx::RenderCommandEncoder>& encoder, const ShaderData& shader_data) {
        if (mInstanceCount == 0) {
            return;
        }

        encoder->setDepthStencilState(mDepthStencilState);
        encoder->setRenderPipelineState(mRenderPipelineState);
        encoder->pushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(ShaderData), &shader_data);

        encoder->bindIndexBuffer(mQuadIndexBuffer, 0, vk::IndexType::eUint32);
        encoder->bindVertexBuffer(0, mQuadVertexBuffer, 0);
        encoder->bindVertexBuffer(1, mInstanceVertexBuffer, 0);
        encoder->drawIndexed(6, mInstanceCount, 0, 0, 0);
    }

    void emit(const glm::vec3 &position, const glm::vec4 &color, const glm::vec3 &velocity, float lifetime) {
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

    auto updateEvent() -> Signal<void(Particle&, float)>& {
        return mParticleUpdateEvent;
    }
};
