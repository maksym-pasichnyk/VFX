#pragma once

#include "Object.hpp"
#include "BlockElement.hpp"

struct ModelLoader;
struct BlockModel : Object {
private:
    bool hasAmbientOcclusion;

    sp<BlockModel> parent;
    std::vector<BlockElement> elements;
    std::map<std::string, std::string> textures;

public:
    BlockModel(bool hasAmbientOcclusion, sp<BlockModel> parent, std::vector<BlockElement> elements, std::map<std::string, std::string> textures);

    auto getElements() const -> std::span<const BlockElement>;
    auto getTexture(const std::string& name) const -> std::string;
    auto getAmbientOcclusion() const -> bool;

    auto getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string>;

private:
    auto lookupTexture(const std::string& texture) const -> std::string;

    static auto isTextureReference(const std::string& texture) -> bool;
};
