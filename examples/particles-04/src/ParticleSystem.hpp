#pragma once

#include "Object.hpp"
#include "Assets.hpp"
#include "Signal.hpp"
#include "Particle.hpp"
#include "Graphics.hpp"

struct ParticleSystem : Object {
private:
    struct Instance {
        glm::vec3 position;
        glm::vec4 color;
    };

private:
    gfx::Device mDevice;
    gfx::Buffer mQuadIndexBuffer;
    gfx::Buffer mQuadVertexBuffer;
    gfx::Buffer mInstanceVertexBuffer;
    gfx::RenderPipelineState mRenderPipelineState;

    size_t mInstanceCount = {};
    std::vector<Particle> mParticles = {};
    std::vector<Instance> mInstances = {};

    Signal<void(Particle&)> mParticleDeathEvent = {};
    Signal<void(Particle&, float)> mParticleUpdateEvent = {};

public:
    explicit ParticleSystem(gfx::Device device, size_t capacity) : mDevice(std::move(device)) {
        mParticles.resize(capacity);
        mInstances.resize(capacity);

        buildShaders();
        buildBuffers();
    }

private:
    void buildShaders() {
        auto vertexLibrary = mDevice.newLibrary(Assets::readFile("shaders/particles.vert.spv"));
        auto fragmentLibrary = mDevice.newLibrary(Assets::readFile("shaders/particles.frag.spv"));

        gfx::RenderPipelineStateDescription description;
        description.vertexFunction = vertexLibrary.newFunction("main");
        description.fragmentFunction = fragmentLibrary.newFunction("main");
        description.vertexInputState = {
            .bindings = {
                vk::VertexInputBindingDescription{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex},
                vk::VertexInputBindingDescription{1, sizeof(Instance), vk::VertexInputRate::eInstance}
            },
            .attributes = {
                vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, 0},
                vk::VertexInputAttributeDescription{1, 1, vk::Format::eR32G32B32Sfloat, offsetof(Instance, position)},
                vk::VertexInputAttributeDescription{2, 1, vk::Format::eR32G32B32A32Sfloat, offsetof(Instance, color)},
            }
        };

        description.colorAttachmentFormats[0] = vk::Format::eB8G8R8A8Unorm;
        description.depthStencilState.depth_test_enable = true;

        description.colorBlendAttachments[0].setBlendEnable(true);
        description.colorBlendAttachments[0].setColorBlendOp(vk::BlendOp::eAdd);
        description.colorBlendAttachments[0].setAlphaBlendOp(vk::BlendOp::eAdd);
        description.colorBlendAttachments[0].setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
        description.colorBlendAttachments[0].setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);

        mRenderPipelineState = mDevice.newRenderPipelineState(description);
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

        mQuadIndexBuffer = mDevice.newBuffer(vk::BufferUsageFlagBits::eIndexBuffer, quadIndices.data(), quadIndices.size() * sizeof(uint32_t), VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
        mQuadVertexBuffer = mDevice.newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, quadVertices.data(), quadVertices.size() * sizeof(glm::vec3), VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
        mInstanceVertexBuffer = mDevice.newBuffer(vk::BufferUsageFlagBits::eVertexBuffer, mInstances.size() * sizeof(Instance), VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
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
            std::memcpy(mInstanceVertexBuffer.contents(), mInstances.data(), mInstanceCount * sizeof(Instance));
            mInstanceVertexBuffer.didModifyRange(0, mInstanceCount * sizeof(Instance));
        }
    }

    auto renderPipelineState() -> gfx::RenderPipelineState {
        return mRenderPipelineState;
    }

    void draw(gfx::CommandBuffer cmd) {
        if (mInstanceCount == 0) {
            return;
        }

        cmd.bindIndexBuffer(mQuadIndexBuffer, 0, vk::IndexType::eUint32);
        cmd.bindVertexBuffer(0, mQuadVertexBuffer, 0);
        cmd.bindVertexBuffer(1, mInstanceVertexBuffer, 0);
        cmd.drawIndexed(6, mInstanceCount, 0, 0, 0);
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
