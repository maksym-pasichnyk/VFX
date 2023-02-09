#include "BlockModelDefinition.hpp"
#include "JsonParser.hpp"
#include "Iter.hpp"

BlockModelDefinition::BlockModelDefinition(sp<MultiPart> multipart, std::map<std::string, sp<MultiVariant>> variants) : multipart(std::move(multipart)), variants(std::move(variants)) {}

auto BlockModelDefinition::getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string> {
    return cxx::iter(variants)
        .flatmap([&](auto& _, auto& multiVariant) -> std::vector<std::string> {
            return multiVariant->getTextureDependencies(modelLoader);
        })
        .collect();
}

auto BlockModelDefinition::fromStream(std::istream& stream) -> sp<BlockModelDefinition> {
    auto object = std::get<JsonObject>(JsonParser::fromStream(stream));
    return sp<BlockModelDefinition>::of(getMultiPart(object), getMultiVariants(object));
}

auto BlockModelDefinition::getMultiPart(const JsonObject& object) -> sp<MultiPart> {
    if (object.contains("multipart")) {
        return MultiPart::deserialize(object.at("multipart"));
    }
    return {};
}

auto BlockModelDefinition::getMultiVariants(const JsonObject& object) -> std::map<std::string, sp<MultiVariant>> {
    if (object.contains("variants")) {
        return cxx::iter(std::get<JsonObject>(object.at("variants")))
            .map([](auto& name, auto& item) {
                return std::pair{name, MultiVariant::deserialize(item)};
            })
            .to<std::map<std::string, sp<MultiVariant>>>();
    }
    return {};
}
