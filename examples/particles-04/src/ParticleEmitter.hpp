#pragma once

#include "ParticleSystem.hpp"

struct ParticleEmitter : ManagedObject<ParticleEmitter> {
protected:
    ManagedShared<ParticleSystem> mParticleSystem;
    float mEmitRate = {};
    float mTime = {};

public:
    ParticleEmitter(ManagedShared<ParticleSystem> particleSystem, float emitRate)
        : mParticleSystem(std::move(particleSystem)), mEmitRate(std::max(emitRate, 0.01F)) {}

public:
    virtual void emit() = 0;

    void update(float dt) {
        mTime += dt;
        while (mTime >= mEmitRate) {
            mTime -= mEmitRate;
            emit();
        }
    }
};
