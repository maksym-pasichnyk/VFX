#include "BlockState.hpp"
#include "Block.hpp"

BlockState::BlockState(sp<Block> owner, std::map<sp<Property>, std::string> values) : owner(std::move(owner)), values(std::move(values)) {}

auto BlockState::getBlock() const -> const sp<Block>& {
    return owner;
}

void BlockState::populateNeighbours(const std::map<std::map<sp<Property>, std::string>, sp<BlockState>>& map) {
    for (auto& [property, value] : values) {
        for (auto& possibleValue : property->getPossibleValues()) {
            if (possibleValue == value) {
                continue;
            }

            auto copy = values;
            copy.insert_or_assign(property, possibleValue);
            neighbours.emplace(std::pair{property, possibleValue}, map.at(copy));
        }
    }
}

auto BlockState::getValue(const sp<Property>& property) -> const std::string& {
    return values.at(property);
}

auto BlockState::setValue(const sp<Property>& property, const std::string& value) -> sp<BlockState> {
    if (values.at(property) == value) {
        return RetainPtr(this);
    }
    return neighbours.at(std::pair{property, value});
}

auto BlockState::setValueByName(const std::string& property, const std::string& value) -> sp<BlockState> {
    return setValue(getBlock()->getStateDefinition()->getProperty(property), value);
}
