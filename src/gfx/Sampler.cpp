#include "Sampler.hpp"

gfx::Sampler::Sampler(rc<Device> device, vk::SamplerCreateInfo const& create_info) : device(std::move(device)) {
    this->handle = this->device->handle.createSampler(create_info, VK_NULL_HANDLE, this->device->dispatcher);
}

gfx::Sampler::~Sampler() {
    this->device->handle.destroySampler(this->handle, VK_NULL_HANDLE, this->device->dispatcher);
}

void gfx::Sampler::setLabel(this Sampler& self, std::string const& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler);
    info.setObject(uint64_t(VkSampler(self.handle)));
    info.setPObjectName(name.c_str());
    self.device->handle.debugMarkerSetObjectNameEXT(info, self.device->dispatcher);
}