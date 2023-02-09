#pragma once

#include <string>

enum class PushReaction {
    NORMAL,
    DESTROY,
    BLOCK,
    IGNORE,
    PUSH_ONLY
};

inline auto getPushReactionByName(const std::string& name) -> PushReaction {
    if (name == "normal") {
        return PushReaction::NORMAL;
    }
    if (name == "destroy") {
        return PushReaction::DESTROY;
    }
    if (name == "block") {
        return PushReaction::BLOCK;
    }
    if (name == "ignore") {
        return PushReaction::IGNORE;
    }
    if (name == "push_only") {
        return PushReaction::PUSH_ONLY;
    }
    throw std::runtime_error("Unknown push reaction: " + name);
}