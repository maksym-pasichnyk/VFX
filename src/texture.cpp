#include "texture.hpp"
#include "context.hpp"
#include "buffer.hpp"
#include "queue.hpp"

void vfx::Texture::setPixelData(std::span<const glm::u8vec4> pixels) {
    auto tmp = context->makeBuffer(BufferUsage::CopySrc, pixels.size_bytes());
    tmp->update(pixels.data(), pixels.size_bytes(), 0);

    const auto barrier_1 = vk::ImageMemoryBarrier{
        .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
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
    };

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

    const auto barrier_2 = vk::ImageMemoryBarrier{
        .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
        .dstAccessMask = vk::AccessFlagBits::eShaderRead,
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
    };

    auto queue = context->makeCommandQueue(1);
    auto cmd = queue->makeCommandBuffer();

    cmd->begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmd->handle.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, {barrier_1});
    cmd->handle.copyBufferToImage(tmp->handle, image, vk::ImageLayout::eTransferDstOptimal, {region});
    cmd->handle.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, {barrier_2});
    cmd->end();
    cmd->submit();
    cmd->waitUntilCompleted();

    context->freeCommandQueue(queue);
    context->freeBuffer(tmp);
}
