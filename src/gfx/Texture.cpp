#include "Device.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"
#include "CommandQueue.hpp"
#include "CommandBuffer.hpp"
#include "ComputePipelineState.hpp"

gfx::Texture::Texture(rc<Device> device, vk::Image image, vk::Format format, vk::Extent3D extent, vk::ImageView image_view, vk::ImageSubresourceRange subresource, VmaAllocation allocation) : device(std::move(device)), image(image), format(format), extent(extent), image_view(image_view), subresource(subresource), allocation(allocation) {}
gfx::Texture::~Texture() {
    device->handle.destroyImageView(image_view, VK_NULL_HANDLE, device->dispatcher);
    if (allocation) {
        vmaDestroyImage(device->allocator, image, allocation);
    }
}

void gfx::Texture::replaceRegion(this Texture& self, const void* data, uint64_t size) {
    auto storageBuffer = self.device->newBuffer(vk::BufferUsageFlagBits::eTransferSrc, data, size, StorageMode::eShared, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    vk::BufferImageCopy buffer_image_copy = {};
    buffer_image_copy.setImageExtent(self.extent);
    buffer_image_copy.imageSubresource.setAspectMask(self.subresource.aspectMask);
    buffer_image_copy.imageSubresource.setLayerCount(1);

    auto commandQueue = self.device->newCommandQueue();
    auto commandBuffer = commandQueue->newCommandBuffer();

    commandBuffer->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    commandBuffer->setImageLayout(self.shared_from_this(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits2::eHost, vk::PipelineStageFlagBits2::eTransfer, {}, vk::AccessFlagBits2::eTransferWrite);
    commandBuffer->handle.copyBufferToImage(storageBuffer->handle, self.image, vk::ImageLayout::eTransferDstOptimal, 1, &buffer_image_copy, self.device->dispatcher);
    commandBuffer->setImageLayout(self.shared_from_this(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead);
    commandBuffer->end();
    commandBuffer->submit();
    commandBuffer->waitUntilCompleted();
}

void gfx::Texture::setLabel(this Texture& self, std::string const& name) {
    vk::DebugMarkerObjectNameInfoEXT image_info = {};
    image_info.setObjectType(vk::DebugReportObjectTypeEXT::eImage);
    image_info.setObject(uint64_t(static_cast<VkImage>(self.image)));
    image_info.setPObjectName(name.c_str());

    self.device->handle.debugMarkerSetObjectNameEXT(image_info, self.device->dispatcher);

    vk::DebugMarkerObjectNameInfoEXT view_info = {};
    view_info.setObjectType(vk::DebugReportObjectTypeEXT::eImageView);
    view_info.setObject(uint64_t(static_cast<VkImageView>(self.image_view)));
    view_info.setPObjectName(name.c_str());

    self.device->handle.debugMarkerSetObjectNameEXT(view_info, self.device->dispatcher);
}

