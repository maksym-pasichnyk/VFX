#pragma once

#include "Object.hpp"
#include "block/PushReaction.hpp"

struct Material : Object {
    struct Builder;

public:
    Material(bool liquid, bool blocks_motion, bool flammable, bool replaceable, bool solid, bool solid_blocking, PushReaction push_reaction)
        : liquid(liquid)
        , blocksMotion(blocks_motion)
        , flammable(flammable)
        , replaceable(replaceable)
        , solid(solid)
        , solidBlocking(solid_blocking)
        , pushReaction(push_reaction) {}

    auto isLiquid() const -> bool {
        return liquid;
    }
    auto isBlocksMotion() const -> bool {
        return blocksMotion;
    }
    auto isFlammable() const -> bool {
        return flammable;
    }
    auto isReplaceable() const -> bool {
        return replaceable;
    }
    auto isSolid() const -> bool {
        return solid;
    }
    auto isSolidBlocking() const -> bool {
        return solidBlocking;
    }
    auto getPushReaction() const -> PushReaction {
        return pushReaction;
    }

private:
    bool liquid;
    bool blocksMotion;
    bool flammable;
    bool replaceable;
    bool solid;
    bool solidBlocking;
    PushReaction pushReaction;
};

struct Material::Builder {
private:
    bool liquid = false;
    bool blocksMotion = true;
    bool flammable = false;
    bool replaceable = false;
    bool solid = true;
    bool solidBlocking = true;
    PushReaction pushReaction = PushReaction::NORMAL;

public:
    auto setLiquid(bool _liquid) -> Builder& {
        liquid = _liquid;
        return *this;
    }
    auto setBlocksMotion(bool _blocksMotion) -> Builder& {
        blocksMotion = _blocksMotion;
        return *this;
    }
    auto setFlammable(bool _flammable) -> Builder& {
        flammable = _flammable;
        return *this;
    }
    auto setReplaceable(bool _replaceable) -> Builder& {
        replaceable = _replaceable;
        return *this;
    }
    auto setSolid(bool _solid) -> Builder& {
        solid = _solid;
        return *this;
    }
    auto setSolidBlocking(bool _solidBlocking) -> Builder& {
        solidBlocking = _solidBlocking;
        return *this;
    }
    auto setPushReaction(PushReaction _pushReaction) -> Builder& {
        pushReaction = _pushReaction;
        return *this;
    }

    auto build() -> sp<Material> {
        return sp<Material>::of(/*this.color, */liquid, blocksMotion, flammable, replaceable, solid, solidBlocking, pushReaction);
    }
};
