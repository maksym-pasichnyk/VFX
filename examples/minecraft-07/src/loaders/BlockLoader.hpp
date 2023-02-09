#pragma once

#include "JsonParser.hpp"
#include "block/Block.hpp"
#include "block/Material.hpp"
#include "registry/ResourceLocation.hpp"
#include "registry/DefaultedRegistry.hpp"

extern sp<MappedRegistry<sp<Block>>> BLOCK;
extern sp<MappedRegistry<sp<Material>>> MATERIAL;

struct BlockLoader {
    static auto getMaterial(const JsonObject& object) -> sp<Material> {
        auto name = getResourceLocation(std::get<std::string>(object.at("material")));
        return MATERIAL->get(name).value();
    }

    static auto deserialize(const JsonElement& json) -> sp<Block> {
        auto& object = std::get<JsonObject>(json);

        auto properties = cxx::iter(std::get<JsonObject>(object.at("properties")))
            .map([&](const std::string& key, const JsonElement& value) {
                auto values = cxx::iter(std::get<JsonArray>(value))
                .map(std::mem_fn(&JsonElement::get<std::string>))
                .template to<std::set<std::string>>();
                return sp<Property>::of(key, std::move(values));
            })
            .collect();

        auto material = getMaterial(object);
        auto block = sp<Block>::of(material);
        block->createBlockStateDefinition(properties);
        return block;
    }

    void reload() {
        cxx::filesystem::iterate_recursive("blocks", [&](const cxx::filesystem::path& path) {
            auto json = JsonParser::fromStream(cxx::ifstream(path));
            BLOCK->registerMapping(path.stem(), deserialize(json));
        });

//        auto resources = std::filesystem::directory_iterator("assets/blocks");
//        cxx::iter(resources)
//            .where([](auto& entry) { return entry.is_regular_file(); })
//            .where([](auto& entry) { return entry.path().extension() == ".json"; })
//            .map([](auto& entry) { return entry.path(); })
//            .for_each([&](const auto& path) {
//                auto stream = std::ifstream(path, std::ios::binary);
//                auto json = JsonParser::fromStream(stream);
//
//                BLOCK->registerMapping(path.stem(), deserialize(json));
//            });
    }
};
