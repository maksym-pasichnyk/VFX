#include "buffer.hpp"
#include "context.hpp"

vfx::Buffer::Buffer() {}

vfx::Buffer::~Buffer() {
    context->freeBuffer(this);
}

void vfx::Buffer::update(const void* src, u64 size, u64 offset) {
    void* ptr = nullptr;
    vmaMapMemory(context->allocator, allocation, &ptr);
    auto dst = static_cast<std::byte*>(ptr) + offset;
    memcpy(dst, src, size);
    vmaUnmapMemory(context->allocator, allocation);
}

void vfx::Buffer::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eBuffer);
    info.setObject(u64(VkBuffer(handle)));
    info.setPObjectName(name.c_str());

    context->device->debugMarkerSetObjectNameEXT(info);
}
