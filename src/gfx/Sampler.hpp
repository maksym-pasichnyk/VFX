#pragma once

#include "Device.hpp"

namespace gfx {
    struct Sampler : public ManagedObject {
        rc<Device>  device;
        vk::Sampler handle;

        explicit Sampler(rc<Device> device, vk::SamplerCreateInfo const& create_info);
        ~Sampler() override;

        void setLabel(this Sampler& self, std::string const& name);
    };
}