#pragma once

#include "Object.hpp"
#include "JsonElement.hpp"

#include <string>

struct Variant : Object {
    std::string model = {};
    int32_t x = 0;
    int32_t y = 0;
    int32_t weight = 0;

    Variant(std::string model, int32_t x, int32_t y, int32_t weight) : model(std::move(model)), x(x), y(y), weight(weight) {}

    auto getModelLocation() const -> const std::string&;

    static auto deserialize(const JsonElement& element) -> sp<Variant> {
        auto& object = std::get<JsonObject>(element);
        auto model = object.at("model").as<std::string>();
        int32_t x = object.value_or("x", 0);
        int32_t y = object.value_or("y", 0);
        int32_t weight = object.value_or("weight", 0);
        return sp<Variant>::of(std::move(model), x, y, weight);
    }
};
