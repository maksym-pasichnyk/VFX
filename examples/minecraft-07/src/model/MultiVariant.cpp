#include "MultiVariant.hpp"
#include "ModelBakery.hpp"
#include "WeightedBakedModel.hpp"

auto MultiVariant::getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string> {
    return cxx::iter(variants).map(std::mem_fn(&Variant::getModelLocation))
        .flatmap([&](const auto& name) {
            return modelLoader(name)->getTextureDependencies(modelLoader);
        })
        .collect();
}

auto MultiVariant::bake(const sp<ModelBakery>& modelBakery, const TextureLoader& textureLoader) -> sp<BakedModel> {
    if (variants.empty()) {
        return {};
    }
    auto builder = WeightedBakedModel::Builder{};
    for (auto& variant : variants) {
        builder.add(modelBakery->bake(textureLoader, variant), variant->weight);
    }
    return builder.build();
}
