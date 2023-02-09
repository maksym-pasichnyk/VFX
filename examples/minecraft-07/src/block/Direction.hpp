#pragma once

#include <array>
#include <string>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

enum class Direction {
    SOUTH,
    NORTH,
    EAST,
    WEST,
    UP,
    DOWN
};

inline auto getDirectionByName(const std::string& name) -> Direction {
    if (name == "south") {
        return Direction::SOUTH;
    }
    if (name == "north") {
        return Direction::NORTH;
    }
    if (name == "east") {
        return Direction::EAST;
    }
    if (name == "west") {
        return Direction::WEST;
    }
    if (name == "up") {
        return Direction::UP;
    }
    if (name == "down" || name == "bottom") {
        return Direction::DOWN;
    }
    throw std::runtime_error("Unknown facing: " + name);
}

inline auto getNormalByDirection(Direction direction) -> glm::ivec3 {
    switch (direction) {
        case Direction::SOUTH: return {0, 0, +1};
        case Direction::NORTH: return {0, 0, -1};
        case Direction::EAST:  return {+1, 0, 0};
        case Direction::WEST:  return {-1, 0, 0};
        case Direction::UP:    return {0, +1, 0};
        case Direction::DOWN:  return {0, -1, 0};
    }
}

inline auto getNearestDirection(const glm::vec3& normal) -> Direction {
    using enum Direction;

    Direction nearestDirection = NORTH;
    float_t maxSquareDistance = std::numeric_limits<float_t>::min();
    for (auto direction : std::array{UP, DOWN, SOUTH, NORTH, EAST, WEST}) {
        float_t squareDistance = glm::dot(glm::vec3(getNormalByDirection(direction)), normal);
        if (squareDistance > maxSquareDistance) {
            nearestDirection = direction;
            maxSquareDistance = squareDistance;
        }
    }
    return nearestDirection;
}

inline auto getOppositeDirection(Direction direction) -> Direction {
    using enum Direction;

    switch (direction) {
        case Direction::SOUTH: return Direction::NORTH;
        case Direction::NORTH: return Direction::SOUTH;
        case Direction::EAST:  return Direction::WEST;
        case Direction::WEST:  return Direction::EAST;
        case Direction::UP:    return Direction::DOWN;
        case Direction::DOWN:  return Direction::UP;
    }
}