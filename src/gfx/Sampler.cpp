#include "Sampler.hpp"
#include "Device.hpp"

gfx::Sampler::Sampler(SharedPtr<Device> device, const vk::SamplerCreateInfo& info) : mDevice(std::move(device)) {
    vkSampler = mDevice->vkDevice.createSampler(info, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}

gfx::Sampler::~Sampler() {
    mDevice->vkDevice.destroySampler(vkSampler, VK_NULL_HANDLE, mDevice->vkDispatchLoaderDynamic);
}

void gfx::Sampler::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler);
    info.setObject(uint64_t(VkSampler(vkSampler)));
    info.setPObjectName(name.c_str());

    mDevice->vkDevice.debugMarkerSetObjectNameEXT(info, mDevice->vkDispatchLoaderDynamic);
}