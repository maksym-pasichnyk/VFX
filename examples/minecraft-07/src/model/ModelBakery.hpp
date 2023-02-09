#pragma once

#include "Iter.hpp"
#include "FaceInfo.hpp"

#include "JsonParser.hpp"
#include "JsonElement.hpp"

#include "BlockModel.hpp"
#include "BakedModel.hpp"
#include "SimpleBakedModel.hpp"
#include "WeightedBakedModel.hpp"
#include "BlockModelDefinition.hpp"

#include "block/Block.hpp"
#include "block/BlockState.hpp"
#include "block/StateDefinition.hpp"

#include "texture/Sprite.hpp"
#include "texture/SpriteAtlas.hpp"
#include "texture/TextureManager.hpp"
#include "registry/ResourceLocation.hpp"
#include "registry/DefaultedRegistry.hpp"

#include <map>
#include <string>
#include <functional>

struct ModelLoader : std::function<sp<BlockModel>(const std::string& name)> {
    using function::function;
};

struct TextureLoader : std::function<sp<Sprite>(const std::string& name)> {
    using function::function;
};

struct ModelBakery : Object {
private:
    std::vector<sp<SpriteAtlas>> atlases = {};
    std::map<std::string, sp<BlockModel>> cache = {};
    std::map<std::string, sp<BakedModel>> bakedTopLevelModels = {};
    std::map<std::string, sp<UnbakedModel>> unbakedTopLevelModels = {};

public:
    void reload(gfx::Device device);
    void getTopLevel(const std::string& name, const sp<StateDefinition<Block>>& stateDefinition);
    void uploadTextures(const sp<TextureManager>& textureManager);
    auto getBakedTopLevelModels() const -> const std::map<std::string, sp<BakedModel>>&;

    auto bake(const TextureLoader& textureLoader, const sp<Variant>& variant) -> sp<BakedModel>;

private:
    auto getModel(const std::string& name) -> sp<BlockModel>;
    auto getParent(const JsonObject& object) -> sp<BlockModel>;
    auto getBlockModel(const std::string& name) -> sp<BlockModel>;

    static auto getModelDefinition(const std::string& name) -> sp<BlockModelDefinition>;
    static auto getPredicate(const sp<StateDefinition<Block>>& stateDefinition, const std::string& raw) -> Predicate<sp<BlockState>>;
    static auto getShape(const BlockElement& element) -> std::array<float_t, 6>;

    static auto getTextures(const JsonObject& object) -> std::map<std::string, std::string>;
    static auto getElements(const JsonObject& object) -> std::vector<BlockElement>;
    static auto getAmbientOcclusion(const JsonObject& object) -> bool;
};
