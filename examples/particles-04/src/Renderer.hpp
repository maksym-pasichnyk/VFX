#pragma once

#include "gfx/GFX.hpp"
#include "glm/glm.hpp"

#include <random>

struct Particle;
struct ParticleSystem;
struct ParticleEmitter;

struct Renderer : gfx::Referencing {
public:
    explicit Renderer(gfx::SharedPtr<gfx::Device> device_);
    ~Renderer() override = default;

public:
    void update(float_t dt);
    void draw(const gfx::SharedPtr<gfx::Swapchain>& swapchain);
    void screenResized(const vk::Extent2D& size);

private:
    void onRocketParticleDeath(Particle& particle);

    void onRocketParticleUpdate(Particle& particle, float_t dt);
    void onSparkleParticleUpdate(Particle& particle, float_t dt);
    void onExplosionParticleUpdate(Particle& particle, float_t dt);

private:
    glm::f32vec2 screenSize = {};
    glm::mat4x4 g_proj_matrix = {};
    glm::mat4x4 g_view_matrix = {};

    gfx::SharedPtr<ParticleSystem> rocketParticleSystem = {};
    gfx::SharedPtr<ParticleSystem> sparkleParticleSystem = {};
    gfx::SharedPtr<ParticleSystem> explosionParticleSystem = {};

    gfx::SharedPtr<ParticleEmitter> rocketParticleEmitter = {};

    gfx::SharedPtr<gfx::Device> device;
    gfx::SharedPtr<gfx::CommandQueue> commandQueue;
    gfx::SharedPtr<gfx::CommandBuffer> commandBuffer;

    std::default_random_engine random_engine = {};
};
