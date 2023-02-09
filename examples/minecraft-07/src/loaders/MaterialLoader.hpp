#pragma once

#include "block/Material.hpp"
#include "JsonParser.hpp"
#include "block/PushReaction.hpp"
#include "registry/DefaultedRegistry.hpp"

extern sp<MappedRegistry<sp<Material>>> MATERIAL;

struct MaterialLoader {
    static auto getPushReaction(const JsonObject& object) {
        return getPushReactionByName(std::get<std::string>(object.at("push_reaction")));
    }

    static auto deserialize(const JsonElement& json) -> sp<Material> {
        auto& object = std::get<JsonObject>(json);

        auto builder = Material::Builder{};
        builder.setLiquid(object.at("liquid").as<bool>());
        builder.setBlocksMotion(object.at("blocks_motion").as<bool>());
        builder.setFlammable(object.at("flammable").as<bool>());
        builder.setReplaceable(object.at("replaceable").as<bool>());
        builder.setSolid(object.at("solid").as<bool>());
        builder.setSolidBlocking(object.at("solid_blocking").as<bool>());
        builder.setPushReaction(getPushReaction(object));
        return builder.build();
    }

    void reload() {
        cxx::filesystem::iterate_recursive("materials", [&](const std::filesystem::path& path) {
            auto json = JsonParser::fromStream(cxx::ifstream(path));
            MATERIAL->registerMapping(path.stem(), deserialize(json));
        });

//        auto resources = std::filesystem::directory_iterator("assets/materials");
//        cxx::iter(resources)
//            .where([](auto& entry) { return entry.is_regular_file(); })
//            .where([](auto& entry) { return entry.path().extension() == ".json"; })
//            .map([](auto& entry) { return entry.path(); })
//            .for_each([&](const auto& path) {
//                auto stream = std::ifstream(path, std::ios::binary);
//                auto json = JsonParser::fromStream(stream);
//
//                MATERIAL->registerMapping(path.stem(), deserialize(json));
//            });
    }
};
