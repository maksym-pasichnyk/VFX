#pragma once

#include "ParticleSystem.hpp"

struct ParticleEmitter : gfx::Referencing {
protected:
    sp<ParticleSystem> mParticleSystem;
    float_t mEmitRate = {};
    float_t mTime = {};

public:
    ParticleEmitter(sp<ParticleSystem> particleSystem, float_t emitRate)
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
