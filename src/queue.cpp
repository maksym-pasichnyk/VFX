#include "queue.hpp"
#include "drawable.hpp"
#include "swapchain.hpp"

void vfx::CommandBuffer::submit() {
    auto submit_info = vk::SubmitInfo{};
    submit_info.setCommandBuffers(handle);
    submit_info.setSignalSemaphores(semaphore);

    queue.submit(submit_info, fence);
}

void vfx::CommandBuffer::present(vfx::Drawable* drawable) {
    auto present_info = vk::PresentInfoKHR{};
    present_info.setWaitSemaphores(semaphore);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&drawable->layer->swapchain);
    present_info.setPImageIndices(&drawable->index);

    vk::Result result = drawable->layer->context.present_queue.presentKHR(present_info);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        drawable->layer->rebuild();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swapchain image");
    }
}

auto vfx::CommandQueue::makeCommandBuffer() -> vfx::CommandBuffer* {
    while (true) {
        for (auto& command_buffer : command_buffers) {
            vk::Result result = context->logical_device.getFenceStatus(command_buffer.fence);
            if (result == vk::Result::eSuccess) {
                std::ignore = context->logical_device.resetFences(1, &command_buffer.fence);
                return &command_buffer;
            }
            if (result == vk::Result::eErrorDeviceLost) {
                throw std::runtime_error(vk::to_string(result));
            }
        }
    }
}
