#pragma once

#include <variant>

template<typename... T>
struct Enum : std::variant<T...> {
    using std::variant<T...>::variant;

    template<typename U>
    [[nodiscard]] auto is() const -> bool {
        return std::holds_alternative<U>(*this);
    }

    template<typename U>
    [[nodiscard]] auto as() -> U& {
        return std::get<U>(*this);
    }

    template<typename U>
    [[nodiscard]] auto as() const -> const U& {
        return std::get<U>(*this);
    }

    template<typename U>
    [[nodiscard]] auto get_if() -> U* {
        return std::get_if<U>(this);
    }

    template<typename U>
    [[nodiscard]] auto get_if() const -> U const* {
        return std::get_if<U>(this);
    }
};