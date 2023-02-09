#pragma once

#include "block/Direction.hpp"
#include "BlockElementFace.hpp"
#include "BlockElementRotation.hpp"

struct BlockElement {
    glm::vec3 from;
    glm::vec3 to;
    std::optional<BlockElementRotation> rotation;
    std::map<Direction, BlockElementFace> faces;
    bool shade;

    explicit BlockElement(const glm::vec3& from, const glm::vec3& to, std::optional<BlockElementRotation> rotation, std::map<Direction, BlockElementFace> faces, bool shade);

private:
    void fillUVs();

public:
    static auto get(const JsonElement& element) -> BlockElement;

private:
    static auto getVec3(const JsonElement& element) -> glm::vec3;
    static auto getShade(const JsonObject& object) -> bool;
    static auto getFaces(const JsonObject& object) -> std::map<Direction, BlockElementFace>;
    static auto getAxis(const JsonObject& object) -> Axis;
    static auto getRotation(const JsonObject& object) -> std::optional<BlockElementRotation>;
};
