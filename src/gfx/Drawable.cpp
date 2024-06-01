#include "Drawable.hpp"

gfx::Drawable::Drawable(vk::SwapchainKHR swapchain, rc<Texture> texture, uint32_t drawableIndex)
    : swapchain(swapchain)
    , texture(std::move(texture))
    , drawableIndex(drawableIndex) {}
