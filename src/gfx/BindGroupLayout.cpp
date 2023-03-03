#include "BindGroupLayout.hpp"

gfx::BindGroupLayoutShared::BindGroupLayoutShared(Device device, vk::DescriptorSetLayout raw, std::vector<vk::DescriptorSetLayoutBinding> bindings)
    : device(std::move(device)), raw(raw), bindings(std::move(bindings)) {}

gfx::BindGroupLayoutShared::~BindGroupLayoutShared() {
    device.shared->raii.raw.destroyDescriptorSetLayout(raw, nullptr, device.shared->raii.dispatcher);
}

gfx::BindGroupLayout::BindGroupLayout() : shared(nullptr) {}
gfx::BindGroupLayout::BindGroupLayout(std::shared_ptr<BindGroupLayoutShared> shared) : shared(std::move(shared)) {}
