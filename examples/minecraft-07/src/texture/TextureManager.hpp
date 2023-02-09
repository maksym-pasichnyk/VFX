#pragma once

#include "Object.hpp"
#include "Graphics.hpp"

#include <map>
#include <string>

struct Texture : Object {
    virtual auto getTexture() -> gfx::Texture = 0;
};

struct TextureManager : Object {
private:
    std::map<std::string, sp<Texture>> byName = {};

public:
    void registerMapping(const std::string& name, const sp<Texture>& texture) {
        byName.insert_or_assign(name, texture);
    }

    auto getTexture(const std::string& name) const -> const sp<Texture>& {
        return byName.at(name);
    }
};