#pragma once

#include "JsonParser.hpp"
#include "block/Block.hpp"
#include "block/Material.hpp"
#include "registry/ResourceLocation.hpp"
#include "registry/DefaultedRegistry.hpp"

struct BlockLoader {
    static auto getMaterial(sp<MappedRegistry<sp<Material>>> MATERIAL, const JsonObject& object) -> sp<Material> {
        auto name = getResourceLocation(std::get<std::string>(object.at("material")));
        return MATERIAL->get(name).value();
    }

    static auto deserialize(sp<MappedRegistry<sp<Material>>> MATERIAL, const JsonElement& json) -> sp<Block> {
        auto& object = std::get<JsonObject>(json);

        auto properties = cxx::iter(std::get<JsonObject>(object.at("properties")))
            .map([&](const std::string& key, const JsonElement& value) {
                auto values = cxx::iter(std::get<JsonArray>(value))
                    .map(std::mem_fn(&JsonElement::get<std::string>))
                    .template to<std::set<std::string>>();
                return sp<Property>::of(key, std::move(values));
            })
            .collect();

        auto material = getMaterial(MATERIAL, object);
        auto block = sp<Block>::of(material);
        block->createBlockStateDefinition(properties);
        return block;
    }

    void reload(sp<MappedRegistry<sp<Block>>> BLOCK, sp<MappedRegistry<sp<Material>>> MATERIAL) {
        cxx::iter(cxx::filesystem::directory_iterator("blocks"))
//            .where([](auto& entry) { return entry.is_regular_file(); })
            .where([](const auto& entry) { return entry.path().extension() == ".json"; })
            .map([](const auto& entry) { return entry.path(); })
            .for_each([&](const auto& path) {
                auto stream = cxx::ifstream(path);
                auto json = JsonParser::fromStream(stream);

                BLOCK->registerMapping(path.stem(), deserialize(MATERIAL, json));
            });
    }
};
