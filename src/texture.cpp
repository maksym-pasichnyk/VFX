#include "texture.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "queue.hpp"

vfx::Sampler::Sampler() {}

vfx::Sampler::~Sampler() {
    device->freeSampler(this);
}

void vfx::Sampler::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT info = {};
    info.setObjectType(vk::DebugReportObjectTypeEXT::eSampler);
    info.setObject(u64(VkSampler(handle)));
    info.setPObjectName(name.c_str());

    device->handle->debugMarkerSetObjectNameEXT(info, device->interface);
}

vfx::Texture::Texture() {}

vfx::Texture::~Texture() {
    device->freeTexture(this);
}

void vfx::Texture::update(const void* data, u64 _size) {
    auto tmp = device->makeBuffer(
        vk::BufferUsageFlagBits::eTransferSrc,
        _size, data,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
    );
    const auto region = vk::BufferImageCopy{
        .imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .layerCount = 1
        },
        .imageExtent = {
            .width = size.width,
            .height = size.height,
            .depth = 1
        }
    };

    auto queue = device->makeCommandQueue();
    auto cmd = queue->makeCommandBufferWithUnretainedReferences();

    cmd->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eHost,
        .srcAccessMask = vk::AccessFlagBits2{},
        .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eTransferDstOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .levelCount = 1,
            .layerCount = 1
        }
    });
    cmd->flushBarriers();
    cmd->handle->copyBufferToImage(tmp->handle, image, vk::ImageLayout::eTransferDstOptimal, 1, &region, device->interface);
    cmd->imageMemoryBarrier(vk::ImageMemoryBarrier2{
        .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
        .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
        .oldLayout = vk::ImageLayout::eTransferDstOptimal,
        .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .levelCount = 1,
            .layerCount = 1
        }
    });
    cmd->flushBarriers();
    cmd->end();
    cmd->submit();
    cmd->waitUntilCompleted();
}

void vfx::Texture::setLabel(const std::string& name) {
    vk::DebugMarkerObjectNameInfoEXT image_info = {};
    image_info.setObjectType(vk::DebugReportObjectTypeEXT::eImage);
    image_info.setObject(u64(VkImage(image)));
    image_info.setPObjectName(name.c_str());

    device->handle->debugMarkerSetObjectNameEXT(image_info, device->interface);

    vk::DebugMarkerObjectNameInfoEXT view_info = {};
    view_info.setObjectType(vk::DebugReportObjectTypeEXT::eImageView);
    view_info.setObject(u64(VkImageView(view)));
    view_info.setPObjectName(name.c_str());

    device->handle->debugMarkerSetObjectNameEXT(view_info, device->interface);
}
