#pragma once

#include "Direction.hpp"

enum class Axis {
    X,
    Y,
    Z
};

inline auto getAxisByName(const std::string& name) -> Axis {
    if (name == "x") {
        return Axis::X;
    }
    if (name == "y") {
        return Axis::Y;
    }
    if (name == "z") {
        return Axis::Z;
    }
    throw std::runtime_error("Unknown axis: " + name);
}

inline auto getAxisByDirection(Direction direction) -> Axis {
    switch (direction) {
        case Direction::SOUTH: return Axis::Z;
        case Direction::NORTH: return Axis::Z;
        case Direction::EAST:  return Axis::X;
        case Direction::WEST:  return Axis::X;
        case Direction::UP:    return Axis::Y;
        case Direction::DOWN:  return Axis::Y;
    }
}