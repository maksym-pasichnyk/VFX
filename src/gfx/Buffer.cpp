#include "Buffer.hpp"
#include "Device.hpp"

gfx::Buffer::Buffer(SharedPtr<Device> device, const vk::BufferCreateInfo& buffer_create_info, const VmaAllocationCreateInfo& allocation_create_info)
: mDevice(std::move(device)) {
    vmaCreateBuffer(
        mDevice->vmaAllocator,
        reinterpret_cast<const VkBufferCreateInfo*>(&buffer_create_info),
        &allocation_create_info,
        reinterpret_cast<VkBuffer*>(&vkBuffer),
        &vmaAllocation,
        nullptr
    );
}

gfx::Buffer::~Buffer() {
    vmaDestroyBuffer(mDevice->vmaAllocator, vkBuffer, vmaAllocation);
}

auto gfx::Buffer::contents() -> void* {
    VmaAllocationInfo allocation_info = {};
    vmaGetAllocationInfo(mDevice->vmaAllocator, vmaAllocation, &allocation_info);
    return allocation_info.pMappedData;
}

auto gfx::Buffer::length() -> vk::DeviceSize {
    VmaAllocationInfo allocation_info = {};
    vmaGetAllocationInfo(mDevice->vmaAllocator, vmaAllocation, &allocation_info);
    return allocation_info.size;
}

auto gfx::Buffer::didModifyRange(vk::DeviceSize offset, vk::DeviceSize size) -> void {
    vmaFlushAllocation(mDevice->vmaAllocator, vmaAllocation, offset, size);
}

void gfx::Buffer::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer);
    info.setObject(uint64_t(VkBuffer(vkBuffer)));
    info.setPObjectName(name.c_str());

    mDevice->vkDevice.debugMarkerSetObjectNameEXT(info, mDevice->vkDispatchLoaderDynamic);
}
