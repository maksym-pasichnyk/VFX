#pragma once

#include "delegate.hpp"

#include <list>
#include <variant>

template <typename... Ts, typename... Fs>
inline constexpr auto match(std::variant<Ts...>& v, Fs&&... fs) -> decltype(auto) {
    struct matches : Fs... {
        using Fs::operator()...;
    };
    return std::visit(matches{ std::forward<Fs>(fs)... }, v);
}

template <typename... Ts, typename... Fs>
inline constexpr auto match(std::variant<Ts...>&& v, Fs&&... fs) -> decltype(auto) {
    struct matches : Fs... {
        using Fs::operator()...;
    };
    return std::visit(matches{ std::forward<Fs>(fs)... }, std::move(v));
}

template <typename... Ts, typename... Fs>
inline constexpr auto match(const std::variant<Ts...>& v, Fs&&... fs) -> decltype(auto) {
    struct matches : Fs... {
        using Fs::operator()...;
    };
    return std::visit(matches{ std::forward<Fs>(fs)... }, std::move(v));
}

template <typename To, typename From>
inline constexpr auto implicit_cast(From&& from) -> To {
    static_assert(std::is_convertible_v<From, To>);
    return std::forward<From>(from);
}

template<auto fn, typename Self, typename R, typename... Args>
inline constexpr auto as_static_function(auto(Self::*)(Args...) -> R) -> auto(*)(Self*, Args...) -> R {
    return [](Self* self, Args... args) -> R {
        return (self->*fn)(args...);
    };
}

template <auto fn>
inline constexpr auto as_static_function() {
    return as_static_function<fn>(fn);
}

template<typename Signature>
struct Signal;

template<typename... Args>
struct Signal<void(Args...)> {
    using delegate = Delegate<void(Args...)>;

    template<auto fn, typename Self>
    inline void connect(Self *self) {
        connect(self, as_static_function<fn>());
    }

    inline void connect(void(*fn)(Args...)) {
        delegates.emplace_back(delegate{fn});
    }

    template<typename T, typename Self>
    inline void connect(T *self, void(*fn)(Self *, Args...)) {
        delegates.emplace_back(delegate{self, fn});
    }

    inline void clear() {
        delegates.clear();
    }

    template<typename... FwdArgs>
    inline void operator()(FwdArgs &&... args) const {
        for (auto fn = delegates.begin(); fn != delegates.end(); ++fn) {
            (*fn)(args...);
        }
    }

    [[nodiscard]] inline explicit operator bool() const {
        return !empty();
    }

    [[nodiscard]] inline auto empty() const -> bool {
        return delegates.empty();
    }

private:
    std::list<delegate> delegates;
};