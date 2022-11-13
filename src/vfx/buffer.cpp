#include "buffer.hpp"
#include "device.hpp"

vfx::Buffer::Buffer() {}

vfx::Buffer::~Buffer() {
    device->freeBuffer(this);
}

auto vfx::Buffer::map() const -> void* {
    void* out = nullptr;
    vmaMapMemory(device->allocator, allocation, &out);
    return out;
}

void vfx::Buffer::unmap() const {
    vmaUnmapMemory(device->allocator, allocation);
}

void vfx::Buffer::update(const void* src, u64 size, u64 offset) const {
    void* ptr = nullptr;
    vmaMapMemory(device->allocator, allocation, &ptr);
    auto dst = static_cast<std::byte*>(ptr) + offset;
    memcpy(dst, src, size);
    vmaUnmapMemory(device->allocator, allocation);
}

void vfx::Buffer::setLabel(const std::string& name) const {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer);
    info.setObject(u64(VkBuffer(handle)));
    info.setPObjectName(name.c_str());

    device->handle->debugMarkerSetObjectNameEXT(info, device->interface);
}
