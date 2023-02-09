#include "Sprite.hpp"
#include "NativeImage.hpp"

#include "glm/common.hpp"

Sprite::Sprite(sp<NativeImage> image, const sp<SpriteAtlas>& atlas, int32_t origin_x, int32_t origin_y, int32_t size_x, int32_t size_y, int32_t texture_size_x, int32_t texture_size_y) : image(std::move(image)), atlas(atlas.get()), originX(origin_x), originY(origin_y), sizeX(size_x), sizeY(size_y), textureSizeX(texture_size_x), textureSizeY(texture_size_y) {
    minU = float_t(origin_x) / float_t(texture_size_x);
    minV = float_t(origin_y) / float_t(texture_size_y);
    maxU = float_t(origin_x + size_x) / float_t(texture_size_x);
    maxV = float_t(origin_y + size_y) / float_t(texture_size_y);
}

void Sprite::copy(std::span<uint32_t> data) {
    size_t start = textureSizeX * originY + originX;

    for (int32_t y = 0; y < sizeY; ++y) {
        for (int32_t x = 0; x < sizeX; ++x) {
            size_t i = start + y * textureSizeX + x;
            data[i] = image->getPixelRGBA(x, y);
        }
    }
}

auto Sprite::getInterpolateU(float_t t) const -> float_t {
    return glm::mix(minU, maxU, t);
}

auto Sprite::getInterpolateV(float_t t) const -> float_t {
    return glm::mix(minV, maxV, t);
}
