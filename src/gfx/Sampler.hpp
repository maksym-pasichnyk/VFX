#pragma once

#include "Device.hpp"

namespace gfx {
    struct SamplerShared {
        Device device;
        vk::Sampler raw;

        explicit SamplerShared();
        explicit SamplerShared(Device device, vk::Sampler raw);

        ~SamplerShared();
    };

    struct Sampler final {
        std::shared_ptr<SamplerShared> shared;

        explicit Sampler() : shared(nullptr) {}
        explicit Sampler(std::shared_ptr<SamplerShared> shared);

        void setLabel(const std::string& name);
    };
}