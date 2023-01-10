#include "Sampler.hpp"
#include "Device.hpp"

gfx::Sampler::Sampler(SharedPtr<Device> device, const vk::SamplerCreateInfo& info) : mDevice(std::move(device)) {
    mSampler = mDevice->mDevice.createSampler(info, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);
}

gfx::Sampler::~Sampler() {
    mDevice->mDevice.destroySampler(mSampler, VK_NULL_HANDLE, mDevice->mDispatchLoaderDynamic);
}

void gfx::Sampler::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler);
    info.setObject(uint64_t(VkSampler(mSampler)));
    info.setPObjectName(name.c_str());

    mDevice->mDevice.debugMarkerSetObjectNameEXT(info, mDevice->mDispatchLoaderDynamic);
}