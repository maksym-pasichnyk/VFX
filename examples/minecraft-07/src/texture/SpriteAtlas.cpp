#include "SpriteAtlas.hpp"
#include "NativeImage.hpp"
#include "TexturePacker.hpp"
#include "registry/ResourceLocation.hpp"
#include "Sprite.hpp"

#include "spdlog/spdlog.h"

static auto load(const std::string& name) -> sp<NativeImage> {
    return NativeImage::read(fmt::format("textures/{}.png", getResourceLocation(name)));
}

static sp<NativeImage> MISSING_IMAGE = [] {
    auto image = sp<NativeImage>::of(16, 16, 4);

    for (int32_t y = 0; y < 16; ++y) {
        for (int32_t x = 0; x < 16; ++x) {
            if ((y < 8) ^ (x < 8)) {
                image->setPixelRGBA(x, y, -524040);
            } else {
                image->setPixelRGBA(x, y, -16777216);
            }
        }
    }

    return image;
}();

SpriteAtlas::SpriteAtlas(std::string path) : path(std::move(path)) {}

void SpriteAtlas::pack(const std::vector<std::string>& resources) {
    auto thiz = RetainPtr(this);

    sprites.clear();

    TexturePacker packer = {};
    for (const auto& name : std::set(resources.begin(), resources.end())) {
        packer.emplace(getResourceLocation(name), load(name));
    }
    packer.emplace("missing", MISSING_IMAGE);

    packer.pack(textureSizeX, textureSizeY);
    packer.gather([&](auto& name, auto& image, int32_t x, int32_t y, int32_t w, int32_t h) {
        sprites.emplace(name, sp<Sprite>::of(image, thiz, x, y, w, h, textureSizeX, textureSizeY));
    });

    missing = sprites.at("missing");
}

void SpriteAtlas::reload(gfx::Device device) {
    std::vector<uint32_t> data = {};
    data.resize(textureSizeX * textureSizeY, 0xFFFFFFFF);
    for (auto& [_, sprite] : sprites) {
        sprite->copy(data);
    }
    texture = device.newTexture(gfx::TextureSettings{
        .width = uint32_t(textureSizeX),
        .height = uint32_t(textureSizeY),
        .format = vk::Format::eR8G8B8A8Unorm,
        .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
    });
    texture.replaceRegion(data.data(), std::span(data).size_bytes());
}

auto SpriteAtlas::getSprite(const std::string& name) -> sp<Sprite> {
    auto resourceLocation = getResourceLocation(name);
    auto it = sprites.find(resourceLocation);
    if (it != sprites.end()) {
        return it->second;
    }
    spdlog::warn("Missing sprite: {}", name);
    return missing;
}

auto SpriteAtlas::getTexture() -> gfx::Texture {
    return texture;
}

auto SpriteAtlas::getPath() const -> const std::string& {
    return path;
}
