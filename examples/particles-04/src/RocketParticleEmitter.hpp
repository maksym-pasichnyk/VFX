#pragma once

#include "ParticleEmitter.hpp"

#include <random>

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

        float lifetime = std::uniform_real_distribution(1.0F, 2.0F)(random_engine);

        mParticleSystem->emit({}, color, velocity * 5.0F, lifetime);
    }
};
