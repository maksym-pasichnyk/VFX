#pragma once

#include "Object.hpp"
#include "block/Direction.hpp"

struct FaceInfo {
    struct VertexInfo {
        int32_t xFace;
        int32_t yFace;
        int32_t zFace;
    };
    using VertexArray = std::array<VertexInfo, 4>;

    static constexpr auto MIN_X = int32_t(Direction::WEST);
    static constexpr auto MAX_X = int32_t(Direction::EAST);
    static constexpr auto MIN_Y = int32_t(Direction::DOWN);
    static constexpr auto MAX_Y = int32_t(Direction::UP);
    static constexpr auto MIN_Z = int32_t(Direction::NORTH);
    static constexpr auto MAX_Z = int32_t(Direction::SOUTH);

    static constexpr auto DOWN = VertexArray{
        VertexInfo{MIN_X, MIN_Y, MAX_Z},
        VertexInfo{MIN_X, MIN_Y, MIN_Z},
        VertexInfo{MAX_X, MIN_Y, MIN_Z},
        VertexInfo{MAX_X, MIN_Y, MAX_Z}
    };
    static constexpr auto UP = VertexArray{
        VertexInfo{MIN_X, MAX_Y, MIN_Z},
        VertexInfo{MIN_X, MAX_Y, MAX_Z},
        VertexInfo{MAX_X, MAX_Y, MAX_Z},
        VertexInfo{MAX_X, MAX_Y, MIN_Z}
    };
    static constexpr auto NORTH = VertexArray{
        VertexInfo{MAX_X, MAX_Y, MIN_Z},
        VertexInfo{MAX_X, MIN_Y, MIN_Z},
        VertexInfo{MIN_X, MIN_Y, MIN_Z},
        VertexInfo{MIN_X, MAX_Y, MIN_Z}
    };
    static constexpr auto SOUTH = VertexArray{
        VertexInfo{MIN_X, MAX_Y, MAX_Z},
        VertexInfo{MIN_X, MIN_Y, MAX_Z},
        VertexInfo{MAX_X, MIN_Y, MAX_Z},
        VertexInfo{MAX_X, MAX_Y, MAX_Z}
    };
    static constexpr auto WEST = VertexArray{
        VertexInfo{MIN_X, MAX_Y, MIN_Z},
        VertexInfo{MIN_X, MIN_Y, MIN_Z},
        VertexInfo{MIN_X, MIN_Y, MAX_Z},
        VertexInfo{MIN_X, MAX_Y, MAX_Z}
    };
    static constexpr auto EAST = VertexArray{
        VertexInfo{MAX_X, MAX_Y, MAX_Z},
        VertexInfo{MAX_X, MIN_Y, MAX_Z},
        VertexInfo{MAX_X, MIN_Y, MIN_Z},
        VertexInfo{MAX_X, MAX_Y, MIN_Z}
    };

    static constexpr auto VALUES = [] {
        std::array<VertexArray, 6> out{};
        out[MIN_X] = WEST;
        out[MAX_X] = EAST;
        out[MIN_Y] = DOWN;
        out[MAX_Y] = UP;
        out[MIN_Z] = NORTH;
        out[MAX_Z] = SOUTH;
        return out;
    }();

    static constexpr auto get(Direction direction) -> const VertexArray& {
        return VALUES[size_t(direction)];
    }
};
