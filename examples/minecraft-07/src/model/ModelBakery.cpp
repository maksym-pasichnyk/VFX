#include "ModelBakery.hpp"
#include "ModelManager.hpp"
#include "texture/SpriteAtlas.hpp"

#include "fstream.hpp"

static std::string MISSING_MODEL_MESH = R"("{
    "textures": {
       "particle": "missing",
       "missing": "missing"
    },
    "elements": [
         {
            "from": [ 0, 0, 0 ],
            "to": [ 16, 16, 16 ],
            "faces": {
                "down":  { "uv": [ 0, 0, 16, 16 ], "cullface": "down",  "texture": "#missing" },
                "up":    { "uv": [ 0, 0, 16, 16 ], "cullface": "up",    "texture": "#missing" },
                "north": { "uv": [ 0, 0, 16, 16 ], "cullface": "north", "texture": "#missing" },
                "south": { "uv": [ 0, 0, 16, 16 ], "cullface": "south", "texture": "#missing" },
                "west":  { "uv": [ 0, 0, 16, 16 ], "cullface": "west",  "texture": "#missing" },
                "east":  { "uv": [ 0, 0, 16, 16 ], "cullface": "east",  "texture": "#missing' }
            }
        }
    ]
})";

extern sp<MappedRegistry<sp<Block>>> BLOCK;

ModelBakery::ModelBakery() {
    for (const auto& block : BLOCK->values()) {
        getTopLevel(BLOCK->getKey(block).value(), block->getStateDefinition());
    }

    auto modelLoader = std::bind_front(std::mem_fn(&ModelBakery::getModel), this);
    auto materials = cxx::iter(ranges::views::values(unbakedTopLevelModels))
        .flatmap([&](auto& unbakedModel) {
            return unbakedModel->getTextureDependencies(modelLoader);
        })
        .collect();

    auto atlas = sp<SpriteAtlas>::of("textures/atlas/blocks.png");
    atlas->pack(materials);
    atlas->reload();

    atlases.emplace_back(atlas);
}

void ModelBakery::getTopLevel(const std::string& name, const sp<StateDefinition<Block>>& stateDefinition) {
    try {
        auto path = "states/" + getResourceLocation(name) + ".json";
        auto stream = cxx::ifstream(path);
        auto modelDefinition = BlockModelDefinition::fromStream(stream);

        auto& possibleStates = stateDefinition->getPossibleStates();
        cxx::iter(modelDefinition->variants).for_each([&](auto& key, auto& value) {
            auto predicate = getPredicate(stateDefinition, key);
            for (auto& state : cxx::iter(possibleStates).where(predicate)) {
                auto location = ModelManager::stateToModelLocation(state);
                unbakedTopLevelModels.insert_or_assign(location, value);
            }
        });
    } catch (const std::exception& e) {
        spdlog::error("Error while loading {}: {}", name, e.what());
    }
}

void ModelBakery::uploadTextures(const sp<TextureManager>& textureManager) {
    auto modelBakery = std::bind_front(std::mem_fn(&ModelBakery::bake), this);

    auto atlas = atlases.at(0);
    auto textureLoader = [&](const std::string& name) -> sp<Sprite> {
        return atlas->getSprite(name);
    };

    cxx::iter(unbakedTopLevelModels).for_each([&](auto& name, auto& unbaked) {
        auto baked = unbaked->bake(RetainPtr(this), textureLoader);
        bakedTopLevelModels.insert_or_assign(name, baked);
    });

    textureManager->registerMapping(atlas->getPath(), atlas);
}

auto ModelBakery::getBakedTopLevelModels() const -> const std::map<std::string, sp<BakedModel>>& {
    return bakedTopLevelModels;
}

auto ModelBakery::getModel(const std::string& name) -> sp<BlockModel> {
    if (cache.contains(name)) {
        return cache.at(name);
    }
    auto raw = getBlockModel(name);
    cache.emplace(name, raw);

    return cache.at(name);
}

auto ModelBakery::getBlockModel(const std::string& name) -> sp<BlockModel> {
    auto path = "models/" + getResourceLocation(name) + ".json";
    auto stream = cxx::ifstream(path);
    auto object = std::get<JsonObject>(JsonParser::fromStream(stream));

    return sp<BlockModel>::of(
        getAmbientOcclusion(object),
        getParent(object),
        getElements(object),
        getTextures(object)
    );
}

auto ModelBakery::getParent(const JsonObject& object) -> sp<BlockModel> {
    if (object.contains("parent")) {
        return getModel(object.at("parent").as<std::string>());
    }
    return {};
}

auto ModelBakery::getModelDefinition(const std::string& name) -> sp<BlockModelDefinition> {
    auto path = "states/" + getResourceLocation(name) + ".json";
    auto stream = cxx::ifstream(path);
    return BlockModelDefinition::fromStream(stream);
}

auto ModelBakery::getPredicate(const sp<StateDefinition<Block>>& stateDefinition, const std::string& raw) -> Predicate<sp<BlockState>> {
    auto properties = cxx::iter(raw)
        .split(',')
        .map(ranges::to<std::string>())
        .map([](const auto& item) {
            return cxx::iter(item).split('=').map(ranges::to<std::string>()).collect();
        })
        .where([](const auto& parts) { return !parts.empty(); })
        .map([&](const auto& parts) {
            auto property = stateDefinition->getProperty(parts.at(0));
            auto value = parts.at(1);//getPropertyValue(property, parts.at(1));
            return std::pair{property, value};
        })
        .to<std::map<sp<Property>, std::string>>();

    if (properties.empty()) {
        return [](const sp<BlockState>& state) {
            return true;
        };
    }

    return [properties = std::move(properties)](const sp<BlockState>& state) {
        return cxx::iter(properties).all([state](auto& property, auto& value) {
            return state->getValue(property) == value;
        });
    };
}
auto ModelBakery::getShape(const BlockElement& element) -> std::array<float_t, 6> {
    std::array<float, 6> shape = {};
    shape[FaceInfo::MIN_X] = element.from.x;
    shape[FaceInfo::MIN_Y] = element.from.y;
    shape[FaceInfo::MIN_Z] = element.from.z;
    shape[FaceInfo::MAX_X] = element.to.x;
    shape[FaceInfo::MAX_Y] = element.to.y;
    shape[FaceInfo::MAX_Z] = element.to.z;
    return shape;
}

auto ModelBakery::bake(const TextureLoader& textureLoader, const sp<Variant>& variant) -> sp<BakedModel> {
    auto raw = getModel(variant->model);
    auto builder = SimpleBakedModel::Builder(raw->getAmbientOcclusion());

    for (auto& element : raw->getElements()) {
        auto shape = getShape(element);

        for (auto& [facing, face] : element.faces) {
            auto& vertexArray = FaceInfo::get(facing);

            auto uvs = std::array{
                glm::vec2(face.getU(0), face.getV(0)),
                glm::vec2(face.getU(1), face.getV(1)),
                glm::vec2(face.getU(2), face.getV(2)),
                glm::vec2(face.getU(3), face.getV(3))
            };

            auto sprite = textureLoader(raw->getTexture(face.texture));
            auto normal = getNormalByDirection(facing);

//            spdlog::info("uv: {}, {}, {}, {}",
//                sprite->getInterpolateU(0) * sprite->getTextureSizeX(),
//                sprite->getInterpolateV(0) * sprite->getTextureSizeY(),
//                sprite->getInterpolateU(1) * sprite->getTextureSizeX(),
//                sprite->getInterpolateV(1) * sprite->getTextureSizeY()
//            );

            std::array<BakedVertex, 4> vertices = {};
            for (size_t i = 0; i < 4; ++i) {
                auto texcoord = glm::vec2(
                    sprite->getInterpolateU(uvs[i].x * 0.0625F),
                    sprite->getInterpolateV(uvs[i].y * 0.0625F)
                );

                auto position = glm::vec3(
                    shape[vertexArray[i].xFace] * 0.0625F,
                    shape[vertexArray[i].yFace] * 0.0625F,
                    shape[vertexArray[i].zFace] * 0.0625F
                );

                auto color = glm::vec4(1, 1, 1, 1);

                vertices[i].position = position;
                vertices[i].normal = normal;
                vertices[i].color = color;
                vertices[i].texcoord = texcoord;
            }
            if (face.cullForDirection.has_value()) {
                builder.addCulledFace(*face.cullForDirection, BakedQuad{vertices});
            } else {
                builder.addUnculledFace(BakedQuad{vertices});
            }
        }
    }

    return builder.build();
}

auto ModelBakery::getTextures(const JsonObject& object) -> std::map<std::string, std::string> {
    if (!object.contains("textures")) {
        return {};
    }
    return cxx::iter(std::get<JsonObject>(object.at("textures")))
        .map([](auto& key, auto& value) {
            return std::pair{key, value.template as<std::string>()};
        })
        .to<std::map<std::string, std::string>>();
}

auto ModelBakery::getElements(const JsonObject& object) -> std::vector<BlockElement> {
    if (!object.contains("elements")) {
        return {};
    }

    return cxx::iter(std::get<JsonArray>(object.at("elements")))
        .map(&BlockElement::get)
        .collect();
}

auto ModelBakery::getAmbientOcclusion(const JsonObject& object) -> bool {
    return object.value_or("ambientocclusion", true);
}
