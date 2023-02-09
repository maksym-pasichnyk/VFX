#pragma once

#include "Object.hpp"

struct BakedModel;
struct ModelBakery;
struct ModelLoader;
struct TextureLoader;

struct UnbakedModel : Object {
    virtual ~UnbakedModel() = default;

    virtual auto getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string> = 0;
    virtual auto bake(const sp<ModelBakery>& modelBakery, const TextureLoader& textureLoader) -> sp<BakedModel> = 0;
};