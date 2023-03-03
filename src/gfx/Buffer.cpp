#include "Device.hpp"
#include "Buffer.hpp"

gfx::BufferShared::BufferShared(gfx::Device device) : device(std::move(device)), raw(nullptr), allocation(nullptr) {}
gfx::BufferShared::BufferShared(gfx::Device device, vk::Buffer raw, VmaAllocation allocation) : device(std::move(device)), raw(raw), allocation(allocation) {}
gfx::BufferShared::~BufferShared() {
    vmaDestroyBuffer(device.shared->allocator, raw, allocation);
}

auto gfx::Buffer::contents() -> void* {
    VmaAllocationInfo allocation_info = {};
    vmaGetAllocationInfo(shared->device.shared->allocator, shared->allocation, &allocation_info);
    return allocation_info.pMappedData;
}

auto gfx::Buffer::length() -> vk::DeviceSize {
    VmaAllocationInfo allocation_info = {};
    vmaGetAllocationInfo(shared->device.shared->allocator, shared->allocation, &allocation_info);
    return allocation_info.size;
}

auto gfx::Buffer::didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void {
    vmaFlushAllocation(shared->device.shared->allocator, shared->allocation, offset, size);
}

void gfx::Buffer::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer);
    info.setObject(uint64_t(VkBuffer(shared->raw)));
    info.setPObjectName(name.c_str());

    shared->device.shared->raii.raw.debugMarkerSetObjectNameEXT(info, shared->device.shared->raii.dispatcher);
}

gfx::Buffer::Buffer() : shared(nullptr) {}
gfx::Buffer::Buffer(std::shared_ptr<BufferShared> shared) : shared(std::move(shared)) {}
