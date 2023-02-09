#pragma once

#include "Object.hpp"
#include "MultiPart.hpp"
#include "JsonElement.hpp"
#include "MultiVariant.hpp"

struct BlockModelDefinition : Object {
    sp<MultiPart> multipart;
    std::map<std::string, sp<MultiVariant>> variants;

    explicit BlockModelDefinition(sp<MultiPart> multipart, std::map<std::string, sp<MultiVariant>> variants);

    auto getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string>;

    static auto fromStream(std::istream& stream) -> sp<BlockModelDefinition>;

private:
    static auto getMultiPart(const JsonObject& object) -> sp<MultiPart>;
    static auto getMultiVariants(const JsonObject& object) -> std::map<std::string, sp<MultiVariant>>;
};
