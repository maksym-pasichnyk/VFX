#pragma once

#include "Delegate.hpp"

#include <list>
#include <variant>

template <typename... Ts, typename... Fs>
constexpr auto match(std::variant<Ts...>& v, Fs&&... fs) -> decltype(auto) {
    struct matches : Fs... {
        using Fs::operator()...;
    };
    return std::visit(matches{ std::forward<Fs>(fs)... }, v);
}

template <typename... Ts, typename... Fs>
constexpr auto match(std::variant<Ts...>&& v, Fs&&... fs) -> decltype(auto) {
    struct matches : Fs... {
        using Fs::operator()...;
    };
    return std::visit(matches{ std::forward<Fs>(fs)... }, std::move(v));
}

template <typename... Ts, typename... Fs>
constexpr auto match(const std::variant<Ts...>& v, Fs&&... fs) -> decltype(auto) {
    struct matches : Fs... {
        using Fs::operator()...;
    };
    return std::visit(matches{ std::forward<Fs>(fs)... }, std::move(v));
}

template <typename To, typename From>
constexpr auto implicit_cast(From&& from) -> To {
    static_assert(std::is_convertible_v<From, To>);
    return std::forward<From>(from);
}

template<auto fn, typename Self, typename R, typename... Args>
constexpr auto as_static_function(auto(Self::*)(Args...) -> R) -> auto(*)(Self*, Args...) -> R {
    return [](Self* self, Args... args) -> R {
        return (self->*fn)(args...);
    };
}

template <auto fn>
constexpr auto as_static_function() {
    return as_static_function<fn>(fn);
}

template<typename Signature>
struct Signal;

template<typename... Args>
struct Signal<void(Args...)> {
    using delegate = Delegate<void(Args...)>;

    template<auto fn, typename Self>
    void connect(Self *self) {
        connect(self, as_static_function<fn>());
    }

    void connect(void(*fn)(Args...)) {
        mDelegates.emplace_back(delegate{fn});
    }

    template<typename T, typename Self>
    void connect(T *self, void(*fn)(Self *, Args...)) {
        mDelegates.emplace_back(delegate{self, fn});
    }

    void clear() {
        mDelegates.clear();
    }

    template<typename... FwdArgs>
    void operator()(FwdArgs &&... args) const {
        for (auto fn = mDelegates.begin(); fn != mDelegates.end(); ++fn) {
            (*fn)(args...);
        }
    }

    explicit operator bool() const {
        return !empty();
    }

    auto empty() const -> bool {
        return mDelegates.empty();
    }

private:
    std::list<delegate> mDelegates;
};