#pragma once

#include "Object.hpp"

namespace gfx {
    struct Texture;
    struct Swapchain;
    struct CommandBuffer;
    struct Drawable final : Referencing<Drawable> {
        friend Swapchain;
        friend CommandBuffer;

    private:
        Swapchain* pLayer = {};
        SharedPtr<Texture> mTexture = {};
        uint32_t mDrawableIndex = {};

    private:
        Drawable(Swapchain* pLayer, SharedPtr<Texture> texture, uint32_t drawableIndex);

    public:
        auto texture() -> SharedPtr<Texture>;
        auto drawableIndex() -> uint32_t;
    };

}