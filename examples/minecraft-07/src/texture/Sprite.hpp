#pragma once

#include "Object.hpp"
#include "Graphics.hpp"

struct NativeImage;
struct SpriteAtlas;
struct Sprite : Object {
private:
    sp<NativeImage> image;
    SpriteAtlas* atlas;

    int32_t originX;
    int32_t originY;
    int32_t sizeX;
    int32_t sizeY;

    int32_t textureSizeX;
    int32_t textureSizeY;

    float_t minU;
    float_t minV;
    float_t maxU;
    float_t maxV;

public:
    Sprite(sp<NativeImage> image, const sp<SpriteAtlas>& atlas, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y, int32_t texture_size_x, int32_t texture_size_y);

    void copy(std::span<uint32_t> data);

    auto getInterpolateU(float_t t) const -> float_t;
    auto getInterpolateV(float_t t) const -> float_t;

    auto getTextureSizeX() const -> float_t {
        return textureSizeX;
    }
    auto getTextureSizeY() const -> float_t {
        return textureSizeY;
    }
};