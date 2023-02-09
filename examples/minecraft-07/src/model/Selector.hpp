#pragma once

#include "Object.hpp"
#include "model/Condition.hpp"
#include "model/MultiVariant.hpp"

struct Selector : Object {
    Condition condition;
    sp<MultiVariant> multiVariant;

    Selector(Condition condition, sp<MultiVariant> multiVariant) : condition(std::move(condition)), multiVariant(std::move(multiVariant)) {}
};
