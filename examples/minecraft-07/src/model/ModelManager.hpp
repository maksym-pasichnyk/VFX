#pragma once

#include "Iter.hpp"
#include "Object.hpp"
#include "model/ModelBakery.hpp"

#include <string>

struct BlockState;

struct ModelManager : Object {
private:
    sp<BakedModel> missing = {};
    sp<ModelBakery> modelBakery = {};
    sp<TextureManager> textureManager = {};
    std::map<sp<BlockState>, sp<BakedModel>> cache = {};

public:
    explicit ModelManager(const sp<TextureManager>& textureManager);

    void reload(sp<MappedRegistry<sp<Block>>> BLOCK, gfx::Device device);
    auto getModel(const std::string& name) const -> sp<BakedModel>;
    auto getModel(const sp<BlockState>& state) const -> sp<BakedModel>;

public:
    static auto stateToModelLocation(sp<MappedRegistry<sp<Block>>> BLOCK, const sp<BlockState>& state) -> std::string;
};