#include "Drawable.hpp"
#include "Swapchain.hpp"

gfx::SwapchainShared::SwapchainShared(Device device, Surface surface, vk::SwapchainKHR raw, std::vector<Drawable> drawables) : device(std::move(device)), surface(std::move(surface)), raw(raw), drawables(std::move(drawables)) {}
gfx::SwapchainShared::~SwapchainShared() {
    device.handle().destroySwapchainKHR(raw, nullptr, device.dispatcher());
}

auto gfx::Swapchain::drawableSize() -> vk::Extent2D {
    auto extent = shared->drawables[0].texture.shared->extent;
    return vk::Extent2D().setWidth(extent.width).setHeight(extent.height);
}

auto gfx::Swapchain::nextDrawable() -> Drawable {
    auto fence = shared->device.handle().createFenceUnique(vk::FenceCreateInfo(), nullptr, shared->device.dispatcher());

    uint32_t index;
    vk::Result result = shared->device.handle().acquireNextImageKHR(
        shared->raw,
        std::numeric_limits<uint64_t>::max(),
        nullptr,
        *fence,
        &index,
        shared->device.dispatcher()
    );
    if (result != vk::Result::eErrorOutOfDateKHR && result != vk::Result::eSuboptimalKHR && result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vk::resultCheck(shared->device.handle().waitForFences(1, &*fence, true, std::numeric_limits<uint64_t>::max(), shared->device.dispatcher()), "waitForFences");

    return shared->drawables[uint64_t(index)];
}