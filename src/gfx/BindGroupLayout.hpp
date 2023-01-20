#pragma once

#include "Device.hpp"

namespace gfx {
    struct BindGroupLayoutShared {
        Device device;
        vk::DescriptorSetLayout raw;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;

        explicit BindGroupLayoutShared(Device device, vk::DescriptorSetLayout raw, std::vector<vk::DescriptorSetLayoutBinding> bindings);
        ~BindGroupLayoutShared();
    };

    struct BindGroupLayout final {
        std::shared_ptr<BindGroupLayoutShared> shared;

        explicit BindGroupLayout();
        explicit BindGroupLayout(std::shared_ptr<BindGroupLayoutShared> shared);
    };
}