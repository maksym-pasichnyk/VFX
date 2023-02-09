#include "BlockElement.hpp"

BlockElement::BlockElement(const glm::vec3& from, const glm::vec3& to, std::optional<BlockElementRotation> rotation, std::map<Direction, BlockElementFace> faces, bool shade) : from(from), to(to), rotation(rotation), faces(std::move(faces)), shade(shade) {
    fillUVs();
}

void BlockElement::fillUVs() {
    for (auto& [facing, face] : faces) {
        if (!face.uv.empty()) {
            continue;
        }
        face.uv = [&] {
            using enum Direction;
            switch (facing) {
                case DOWN:  return std::vector<float_t>{from.x, 16.0F - to.z, to.x, 16.0F - from.z};
                case UP:    return std::vector<float_t>{from.x, from.z, to.x, to.z};
                case NORTH: return std::vector<float_t>{16.0F - to.x, 16.0F - to.y, 16.0F - from.x, 16.0F - from.y};
                case SOUTH: return std::vector<float_t>{from.x, 16.0F - to.y, to.x, 16.0F - from.y};
                case WEST:  return std::vector<float_t>{from.z, 16.0F - to.y, to.z, 16.0F - from.y};
                case EAST:  return std::vector<float_t>{16.0F - to.z, 16.0F - to.y, 16.0F - from.z, 16.0F - from.y};
            }
        }();
    }
}

auto BlockElement::get(const JsonElement& element) -> BlockElement {
    auto& object = std::get<JsonObject>(element);
    return BlockElement(
        getVec3(object.at("from")),
        getVec3(object.at("to")),
        getRotation(object),
        getFaces(object),
        getShade(object)
    );
}

auto BlockElement::getVec3(const JsonElement& element) -> glm::vec3 {
    auto& array = std::get<JsonArray>(element);
    return {
        array.at(0).get<float_t>(),
        array.at(1).get<float_t>(),
        array.at(2).get<float_t>()
    };
}

auto BlockElement::getShade(const JsonObject& object) -> bool {
    return object.value_or("shade", true);
}

auto BlockElement::getFaces(const JsonObject& object) -> std::map<Direction, BlockElementFace> {
    return cxx::iter(std::get<JsonObject>(object.at("faces")))
        .map([&](auto& key, auto& value) {
            return std::pair{getDirectionByName(key), BlockElementFace::get(value)};
        })
        .to<std::map<Direction, BlockElementFace>>();
}

auto BlockElement::getAxis(const JsonObject& object) -> Axis {
    return getAxisByName(std::get<std::string>(object.at("axis")));
}

auto BlockElement::getRotation(const JsonObject& object) -> std::optional<BlockElementRotation> {
    if (!object.contains("rotation")) {
        return std::nullopt;
    }

    auto rotation = std::get<JsonObject>(object.at("rotation"));
    auto origin = getVec3(rotation.at("origin")) * 0.0625F;
    auto axis = getAxis(rotation);
    auto angle = rotation.at("angle").get<float_t>();
    auto rescale = rotation.value_or("rescale", false);

    return BlockElementRotation(origin, axis, angle, rescale);
}
