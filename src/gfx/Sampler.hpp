#pragma once

#include "Device.hpp"

namespace gfx {
    struct Sampler : ManagedObject<Sampler> {
        ManagedShared<Device>   device;
        vk::Sampler             raw;

        explicit Sampler();
        explicit Sampler(ManagedShared<Device> device, vk::Sampler raw);
        ~Sampler() override;

        void setLabel(const std::string& name);
    };
}