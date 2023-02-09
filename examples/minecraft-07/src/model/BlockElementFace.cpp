#include "BlockElementFace.hpp"
#include "Iter.hpp"

BlockElementFace::BlockElementFace(std::optional<Direction> cullForDirection, int tintIndex, std::string texture, std::vector<float_t> uv, int32_t rotation)
: cullForDirection(cullForDirection), tintIndex(tintIndex), texture(std::move(texture)), uv(std::move(uv)), rotation(rotation) {}

auto BlockElementFace::getU(int32_t index) const -> float_t {
    int32_t i = getShiftedIndex(index);
    if (i == 0 || i == 1) {
        return uv.at(0);
    } else {
        return uv.at(2);
    }
}

auto BlockElementFace::getV(int32_t index) const -> float_t {
    int32_t i = getShiftedIndex(index);
    if (i == 0 || i == 3) {
        return uv.at(1);
    } else {
        return uv.at(3);
    }
}

auto BlockElementFace::getShiftedIndex(int32_t index) const -> int32_t {
    return (index + rotation) % 4;
}

auto BlockElementFace::getReverseIndex(int32_t index) const -> int32_t {
    return (index - rotation + 4) % 4;
}

auto BlockElementFace::get(const JsonElement& element) -> BlockElementFace {
    auto& object = std::get<JsonObject>(element);
    return BlockElementFace(
        getCullFacing(object),
        getTintIndex(object),
        getTexture(object),
        getUV(object),
        getRotation(object)
    );
}

auto BlockElementFace::getUV(const JsonObject& object) -> std::vector<float_t> {
    if (!object.contains("uv")) {
        return {};
    }
    auto& array = object.at("uv").as<JsonArray>();
    if (array.size() != 4) {
        throw std::runtime_error("Expected 4 uv values, found: " + std::to_string(array.size()));
    }
    return cxx::iter(array).map(std::mem_fn(&JsonElement::get<float_t>)).collect();
}

auto BlockElementFace::getRotation(const JsonObject& object) -> int32_t {
    return object.value_or("rotation", 0);
}

auto BlockElementFace::getTintIndex(const JsonObject& object) -> int32_t {
    return object.value_or("tintindex", -1);
}

auto BlockElementFace::getTexture(const JsonObject& object) -> std::string {
    return object.at("texture").as<std::string>();
}

auto BlockElementFace::getCullFacing(const JsonObject& object) -> std::optional<Direction> {
    if (!object.contains("cullface")) {
        return std::nullopt;
    }
    return getDirectionByName(std::get<std::string>(object.at("cullface")));
}
