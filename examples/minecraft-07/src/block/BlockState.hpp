#pragma once

#include "Axis.hpp"
#include "Object.hpp"
#include "Property.hpp"
#include "Direction.hpp"

#include <map>
#include <string>
#include <vector>

struct Block;
struct BlockState : Object {
    sp<Block> owner;
    std::map<sp<Property>, std::string> values = {};
    std::map<std::pair<sp<Property>, std::string>, sp<BlockState>> neighbours = {};

    explicit BlockState(sp<Block> owner, std::map<sp<Property>, std::string> values);

    auto getBlock() const -> const sp<Block>&;
    void populateNeighbours(const std::map<std::map<sp<Property>, std::string>, sp<BlockState>>& map);

    auto getValue(const sp<Property>& property) -> const std::string&;
    auto setValue(const sp<Property>& property, const std::string& value) -> sp<BlockState>;
    auto setValueByName(const std::string& property, const std::string& value) -> sp<BlockState>;
};
