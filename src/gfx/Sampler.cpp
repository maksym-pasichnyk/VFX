#include "Sampler.hpp"

gfx::SamplerShared::SamplerShared() : device(nullptr), raw(nullptr) {}
gfx::SamplerShared::SamplerShared(gfx::Device device, vk::Sampler raw) : device(device), raw(raw) {}
gfx::SamplerShared::~SamplerShared() {
    device.shared->raii.raw.destroySampler(raw, VK_NULL_HANDLE, device.shared->raii.dispatcher);
}

void gfx::Sampler::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler);
    info.setObject(uint64_t(VkSampler(shared->raw)));
    info.setPObjectName(name.c_str());

    shared->device.shared->raii.raw.debugMarkerSetObjectNameEXT(info, shared->device.shared->raii.dispatcher);
}

gfx::Sampler::Sampler(std::shared_ptr<SamplerShared> shared) : shared(std::move(shared)) {}
