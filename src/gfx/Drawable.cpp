#include "Drawable.hpp"

gfx::Drawable::Drawable() : swapchain(nullptr), texture(), drawableIndex(-1) {}

gfx::Drawable::Drawable(vk::SwapchainKHR swapchain, ManagedShared<Texture> texture, uint32_t drawableIndex)
: swapchain(swapchain), texture(std::move(texture)), drawableIndex(drawableIndex) {}
