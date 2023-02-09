#pragma once

#include "Object.hpp"
#include "TextureManager.hpp"

#include <unordered_map>

struct Sprite;
struct SpriteAtlas : Texture {
public:
    explicit SpriteAtlas(std::string path);

    void pack(const std::vector<std::string>& resources);
    void reload();
    auto getSprite(const std::string& name) -> sp<Sprite>;
    auto getTexture() -> gfx::Texture override;
    auto getPath() const -> const std::string&;

private:
    std::string path;
    sp<Sprite> missing;

    gfx::Texture texture;
    int32_t textureSizeX = 0;
    int32_t textureSizeY = 0;
    std::unordered_map<std::string, sp<Sprite>> sprites = {};
};