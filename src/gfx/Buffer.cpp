#include "Device.hpp"
#include "Buffer.hpp"

gfx::Buffer::Buffer(rc<Device> device, vk::Buffer handle, VmaAllocation allocation) : device(std::move(device)), handle(handle), allocation(allocation) {}
gfx::Buffer::~Buffer() {
    vmaDestroyBuffer(device->allocator, handle, allocation);
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

void gfx::Buffer::setLabel(std::string const& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer);
    info.setObject(uint64_t(VkBuffer(handle)));
    info.setPObjectName(name.c_str());

    device->handle.debugMarkerSetObjectNameEXT(info, device->dispatcher);
}

auto gfx::Buffer::descriptorInfo() const -> vk::DescriptorBufferInfo {
    return vk::DescriptorBufferInfo{handle, 0, VK_WHOLE_SIZE};
}
