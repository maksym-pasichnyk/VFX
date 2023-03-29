#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"

gfx::Texture::Texture(ManagedShared<Device> device) : device(std::move(device)), image(nullptr), format(vk::Format::eUndefined), extent(), image_view(nullptr), subresource(), allocation(nullptr) {}
gfx::Texture::Texture(ManagedShared<Device> device, vk::Image image, vk::Format format, vk::Extent3D extent, vk::ImageView image_view, vk::ImageSubresourceRange subresource, VmaAllocation allocation) : device(std::move(device)), image(image), format(format), extent(extent), image_view(image_view), subresource(subresource), allocation(allocation) {}
gfx::Texture::~Texture() {
    device->raii.raw.destroyImageView(image_view, VK_NULL_HANDLE, device->raii.dispatcher);
    if (allocation) {
        vmaDestroyImage(device->allocator, image, allocation);
    }
}

void gfx::Texture::replaceRegion(const void* data, uint64_t size) {
    auto storageBuffer = device->newBuffer(vk::BufferUsageFlagBits::eTransferSrc, data, size, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    vk::BufferImageCopy buffer_image_copy = {};
    buffer_image_copy.setImageExtent(extent);
    buffer_image_copy.imageSubresource.setAspectMask(subresource.aspectMask);
    buffer_image_copy.imageSubresource.setLayerCount(1);

    auto commandQueue = device->newCommandQueue();
    auto commandBuffer = commandQueue->commandBuffer();

    commandBuffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    commandBuffer->setImageLayout(MakeShared(this->retain()), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits2::eHost, vk::PipelineStageFlagBits2::eTransfer, {}, vk::AccessFlagBits2::eTransferWrite);
    commandBuffer->raw.copyBufferToImage(storageBuffer->raw, image, vk::ImageLayout::eTransferDstOptimal, 1, &buffer_image_copy, device->raii.dispatcher);
    commandBuffer->setImageLayout(MakeShared(this->retain()), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead);
    commandBuffer->end();
    commandBuffer->submit();
    commandBuffer->waitUntilCompleted();
}

void gfx::Texture::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT image_info = {};
    image_info.setObjectType(vk::DebugReportObjectTypeEXT::eImage);
    image_info.setObject(uint64_t(static_cast<VkImage>(image)));
    image_info.setPObjectName(name.c_str());

    device->raii.raw.debugMarkerSetObjectNameEXT(image_info, device->raii.dispatcher);

    vk::DebugMarkerObjectNameInfoEXT view_info = {};
    view_info.setObjectType(vk::DebugReportObjectTypeEXT::eImageView);
    view_info.setObject(uint64_t(static_cast<VkImageView>(image_view)));
    view_info.setPObjectName(name.c_str());

    device->raii.raw.debugMarkerSetObjectNameEXT(view_info, device->raii.dispatcher);
}

