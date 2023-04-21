#include "Device.hpp"
#include "Buffer.hpp"

gfx::Buffer::Buffer(ManagedShared<Device> device) : device(std::move(device)), raw(nullptr), allocation(nullptr) {}
gfx::Buffer::Buffer(ManagedShared<Device> device, vk::Buffer raw, VmaAllocation allocation) : device(std::move(device)), raw(raw), allocation(allocation) {}
gfx::Buffer::~Buffer() {
    vmaDestroyBuffer(device->allocator, raw, allocation);
}

auto gfx::Buffer::contents() -> void* {
    VmaAllocationInfo allocation_info = {};
    vmaGetAllocationInfo(device->allocator, allocation, &allocation_info);
    return allocation_info.pMappedData;
}

auto gfx::Buffer::length() -> vk::DeviceSize {
    VmaAllocationInfo allocation_info = {};
    vmaGetAllocationInfo(device->allocator, allocation, &allocation_info);
    return allocation_info.size;
}

auto gfx::Buffer::didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void {
    vmaFlushAllocation(device->allocator, allocation, offset, size);
}

void gfx::Buffer::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer);
    info.setObject(uint64_t(VkBuffer(raw)));
    info.setPObjectName(name.c_str());

    device->raii.raw.debugMarkerSetObjectNameEXT(info, device->raii.dispatcher);
}

auto gfx::Buffer::descriptorInfo() const -> vk::DescriptorBufferInfo {
    return vk::DescriptorBufferInfo{raw, 0, VK_WHOLE_SIZE};
}
