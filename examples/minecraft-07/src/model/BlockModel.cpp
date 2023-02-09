#include "BlockModel.hpp"
#include "spdlog/spdlog.h"

#include <set>

BlockModel::BlockModel(bool hasAmbientOcclusion, sp<BlockModel> parent, std::vector<BlockElement> elements, std::map<std::string, std::string> textures)
    : hasAmbientOcclusion(hasAmbientOcclusion), parent(std::move(parent)), elements(std::move(elements)), textures(std::move(textures)) {}

auto BlockModel::getElements() const -> std::span<const BlockElement> {
    if (elements.empty() && parent) {
        return parent->getElements();
    }
    return std::span(elements);
}

auto BlockModel::getTexture(const std::string& name) const -> std::string {
    std::set<std::string> references{};

    auto texture = name;
    while (isTextureReference(texture)) {
        if (references.contains(texture)) {
            spdlog::error("Texture reference loop detected: {}", name);
            return "missing";
        }
        references.emplace(texture);

        texture = lookupTexture(texture.substr(1));
    }
    return texture;
}

auto BlockModel::getAmbientOcclusion() const -> bool {
    return parent ? parent->getAmbientOcclusion() : hasAmbientOcclusion;
}

auto BlockModel::getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string> {
    std::vector<std::string> materials = {};

    for (auto& element : getElements()) {
        for (auto& [_, face] : element.faces) {
            materials.emplace_back(getTexture(face.texture));
        }
    }

    return materials;
}

auto BlockModel::lookupTexture(const std::string& texture) const -> std::string {
    for (auto current = this; current != nullptr; current = &*current->parent) {
        if (auto it = current->textures.find(texture); it != current->textures.end()) {
            return it->second;
        }
    }
    spdlog::warn("Texture '{}' not found", texture);
    return "missing";
}

auto BlockModel::isTextureReference(const std::string& texture) -> bool {
    return texture.starts_with('#');
}
