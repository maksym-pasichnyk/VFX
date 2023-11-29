#pragma once

#include "ParticleSystem.hpp"

struct ParticleEmitter : public ManagedObject {
protected:
    rc<ParticleSystem> mParticleSystem;
    float mEmitRate = {};
    float mTime = {};

public:
    ParticleEmitter(rc<ParticleSystem> particleSystem, float emitRate)
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
