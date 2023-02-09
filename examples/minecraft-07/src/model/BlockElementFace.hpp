#pragma once

#include "Iter.hpp"
#include "JsonElement.hpp"
#include "block/Direction.hpp"

struct BlockElementFace {
    std::optional<Direction> cullForDirection;
    int tintIndex;
    std::string texture;
    std::vector<float_t> uv;
    int32_t rotation;

    explicit BlockElementFace(std::optional<Direction> cullForDirection, int tintIndex, std::string texture, std::vector<float_t> uv, int32_t rotation);

    auto getU(int32_t index) const -> float_t;
    auto getV(int32_t index) const -> float_t;
    auto getShiftedIndex(int32_t index) const -> int32_t;
    auto getReverseIndex(int32_t index) const -> int32_t;

    static auto get(const JsonElement& element) -> BlockElementFace;

private:
    static auto getUV(const JsonObject& object) -> std::vector<float_t>;
    static auto getRotation(const JsonObject& object) -> int32_t;
    static auto getTintIndex(const JsonObject& object) -> int32_t;
    static auto getTexture(const JsonObject& object) -> std::string;
    static auto getCullFacing(const JsonObject& object) -> std::optional<Direction>;
};
