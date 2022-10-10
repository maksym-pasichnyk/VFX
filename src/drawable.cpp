#include "drawable.hpp"
#include "swapchain.hpp"

namespace vfx {
    void Drawable::present(vk::CommandBuffer command_buffer, vk::Fence fence, vk::Semaphore semaphore) {
//        const auto stages = std::array{
//            vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput}
//        };

        auto submit_info = vk::SubmitInfo{};
//        submit_info.setWaitDstStageMask(stages);
        submit_info.setCommandBuffers(command_buffer);
//        submit_info.setWaitSemaphores(image_available_semaphores[current_frame]);
        submit_info.setSignalSemaphores(semaphore);

        layer->context.graphics_queue.submit(submit_info, fence);

        auto present_info = vk::PresentInfoKHR{};
        present_info.setWaitSemaphores(semaphore);
        present_info.setSwapchainCount(1);
        present_info.setPSwapchains(&layer->swapchain);
        present_info.setPImageIndices(&index);
        auto result = layer->context.present_queue.presentKHR(present_info);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            layer->out_of_date = true;
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swapchain image");
        }
    }
}

