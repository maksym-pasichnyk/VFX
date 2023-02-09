#pragma once

#include "Selector.hpp"
#include "Condition.hpp"
#include "Predicate.hpp"
#include "MultiPart.hpp"
#include "../block/BlockState.hpp"
#include "JsonElement.hpp"
#include "UnbakedModel.hpp"
#include "MultiVariant.hpp"
#include "block/StateDefinition.hpp"
#include "MultiPartBakedModel.hpp"

#include <vector>

struct MultiPart : UnbakedModel {
private:
    std::vector<sp<Selector>> selectors;

public:
    explicit MultiPart(std::vector<sp<Selector>> selectors) : selectors(std::move(selectors)) {}

public:
    static auto deserialize(const JsonElement& element) -> sp<MultiPart> {
        auto selectors = cxx::iter(std::get<JsonArray>(element))
            .map([](auto& element) {
                return getSelector(std::get<JsonObject>(element));
            })
            .collect();
        return sp<MultiPart>::of(std::move(selectors));
    }

    auto getTextureDependencies(const ModelLoader& modelLoader) -> std::vector<std::string> override {
        return {};
    }

    auto bake(const sp<ModelBakery>& modelBakery, const TextureLoader& textureLoader) -> sp<BakedModel> override {
        auto builder = MultiPartBakedModel::Builder{};

        return builder.build();
    }

private:
    static auto getSelector(const JsonObject& object) -> sp<Selector> {
        return sp<Selector>::of(getCondition(object), MultiVariant::deserialize(object.at("apply")));
    }

    static auto getCondition(const JsonObject& object) -> Condition {
        if (object.contains("when")) {
            auto& when = std::get<JsonObject>(object.at("when"));
            if (when.empty()) {
                throw std::runtime_error("No elements found in selector");
            }
            if (when.size() == 1) {
                if (when.contains("OR")) {
                    auto conditions = cxx::iter(std::get<JsonArray>(when.at("OR")))
                        .map([](auto& element) {
                            return getCondition(std::get<JsonObject>(element));
                        })
                        .collect();
                    return Condition::any(std::move(conditions));
                }
                if (when.contains("AND")) {
                    auto conditions = cxx::iter(std::get<JsonArray>(when.at("AND")))
                        .map([](auto& element) {
                            return getCondition(std::get<JsonObject>(element));
                        })
                        .collect();
                    return Condition::all(std::move(conditions));
                }
            }
            return Condition::all(cxx::iter(when).map(getKeyValueCondition).collect());
        }
        return [](...) { return [](...) { return true; }; };
    }

    static auto getKeyValueCondition(const std::string& name, const JsonElement& element) -> Condition {
        auto& value = std::get<std::string>(element);
        return [name, value](auto& stateDefinition) -> Predicate<sp<BlockState>> {
            auto property = stateDefinition->getProperty(name);

            auto values = cxx::iter(value).split('|').map(ranges::to<std::string>()).collect();
            if (values.empty()) {
                throw std::runtime_error("Empty value for property " + property->getName());
            }
            if (values.size() == 1) {
                return getBlockStatePredicate(property, values.front());
            }
            auto predicates = cxx::iter(values)
                .map([&] (auto& element) {
                    return getBlockStatePredicate(property, element);
                })
                .collect();

            return [predicates = std::move(predicates)](auto& blockState) {
                return cxx::iter(predicates).any([&](auto& predicate) {
                    return predicate(blockState);
                });
            };
        };
    }

    static auto getBlockStatePredicate(const sp<Property>& property, const std::string& value) -> Predicate<sp<BlockState>> {
        return [property, value](auto& blockState) {
            return blockState->getValue(property) == value;
        };
    }
};