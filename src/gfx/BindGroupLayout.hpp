#pragma once

#include "Device.hpp"

namespace gfx {
    struct BindGroupLayout : ManagedObject<BindGroupLayout> {
        ManagedShared<Device>                       device;
        vk::DescriptorSetLayout                     raw;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        explicit BindGroupLayout(ManagedShared<Device> device, vk::DescriptorSetLayout raw, std::vector<vk::DescriptorSetLayoutBinding> bindings);
        ~BindGroupLayout() override;
    };
}