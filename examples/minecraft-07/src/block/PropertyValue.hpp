#pragma once

#include "Axis.hpp"
#include "Enum.hpp"
#include "Property.hpp"
#include "Direction.hpp"

struct PropertyValue : Enum<bool, int32_t, Axis, Direction> {
    using Enum::Enum;

    constexpr auto operator==(const PropertyValue& other) const noexcept -> bool = default;
    constexpr auto operator!=(const PropertyValue& other) const noexcept -> bool = default;
};

inline auto getPropertyValue(const sp<Property>& property, const std::string& name) -> PropertyValue {
    // todo: implement
    return PropertyValue{};
}
