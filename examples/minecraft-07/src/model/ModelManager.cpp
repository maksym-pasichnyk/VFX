#include "ModelManager.hpp"
#include "block/BlockState.hpp"
#include "registry/DefaultedRegistry.hpp"

#include <string>
#include "spdlog/fmt/fmt.h"

extern sp<MappedRegistry<sp<Block>>> BLOCK;

ModelManager::ModelManager(const sp<TextureManager>& textureManager) : textureManager(textureManager) {}

auto ModelManager::stateToModelLocation(const sp<BlockState>& state) -> std::string {
    auto name = BLOCK->getKey(state->getBlock()).value();
    auto values = cxx::iter(state->values).map([](auto& property, auto& value) {
        return fmt::format("{}={}", property->getName(), value);
    });
    return fmt::format("{}#{}", name, fmt::join(values, ","));
}

void ModelManager::reload() {
    modelBakery = sp<ModelBakery>::of();
    modelBakery->uploadTextures(textureManager);

//    missing = modelBakery->getBakedTopLevelModels().at("missing");

    for (const auto& block : BLOCK->values()) {
        cxx::iter(block->getStateDefinition()->getPossibleStates()).for_each([&](auto& state) {
            cache.insert_or_assign(state, getModel(stateToModelLocation(state)));
        });
    }
}

auto ModelManager::getModel(const std::string& name) const -> sp<BakedModel> {
    auto& bakedTopLevelModels = modelBakery->getBakedTopLevelModels();
    if (auto it = bakedTopLevelModels.find(name); it != bakedTopLevelModels.end()) {
        return it->second;
    }
    return missing;
}


auto ModelManager::getModel(const sp<BlockState>& state) const -> sp<BakedModel> {
    if (auto it = cache.find(state); it != cache.end()) {
        return it->second;
    }
    return missing;
}
