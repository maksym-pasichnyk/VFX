#include "Sampler.hpp"

gfx::Sampler::Sampler() : device(), raw(nullptr) {}
gfx::Sampler::Sampler(ManagedShared<Device> device, vk::Sampler raw) : device(std::move(device)), raw(raw) {}
gfx::Sampler::~Sampler() {
    device->raii.raw.destroySampler(raw, VK_NULL_HANDLE, device->raii.dispatcher);
}

void gfx::Sampler::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler);
    info.setObject(uint64_t(VkSampler(raw)));
    info.setPObjectName(name.c_str());

    device->raii.raw.debugMarkerSetObjectNameEXT(info, device->raii.dispatcher);
}