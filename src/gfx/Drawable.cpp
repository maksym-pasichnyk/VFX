#include "Drawable.hpp"
#include "Texture.hpp"
#include "Swapchain.hpp"

gfx::Drawable::Drawable(Swapchain* layer, SharedPtr<Texture> texture, uint32_t drawableIndex)
: pLayer(layer), mTexture(std::move(texture)), mDrawableIndex(drawableIndex) {}

auto gfx::Drawable::texture() -> SharedPtr<Texture> {
    return mTexture;
}

auto gfx::Drawable::drawableIndex() -> uint32_t {
    return mDrawableIndex;
}