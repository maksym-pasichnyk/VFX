#pragma once

#include "Iter.hpp"
#include "Variant.hpp"
#include "JsonElement.hpp"
#include "UnbakedModel.hpp"

struct MultiVariant : UnbakedModel {
    std::vector<sp<Variant>> variants = {};

    explicit MultiVariant(std::vector<sp<Variant>> variants) : variants(std::move(variants)) {}

    auto getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string> override;

    auto bake(const sp<ModelBakery>& modelBakery, const TextureLoader& textureLoader) -> sp<BakedModel> override;

    static auto deserialize(const JsonElement& element) -> sp<MultiVariant> {
        std::vector<sp<Variant>> variants = {};
        if (element.is<JsonArray>()) {
            variants = cxx::iter(std::get<JsonArray>(element)).map(Variant::deserialize).collect();
        } else {
            variants.emplace_back(Variant::deserialize(element));
        }
        return sp<MultiVariant>::of(std::move(variants));
    }
};
