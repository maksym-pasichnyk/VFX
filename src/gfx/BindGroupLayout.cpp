#include "BindGroupLayout.hpp"

gfx::BindGroupLayout::BindGroupLayout(ManagedShared<Device> device, vk::DescriptorSetLayout raw, std::vector<vk::DescriptorSetLayoutBinding> bindings)
    : device(std::move(device)), raw(raw), bindings(std::move(bindings)) {}

gfx::BindGroupLayout::~BindGroupLayout() {
    device->raii.raw.destroyDescriptorSetLayout(raw, nullptr, device->raii.dispatcher);
}