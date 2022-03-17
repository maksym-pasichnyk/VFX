#pragma once

#include <utility>

template<typename Signature>
struct Delegate;

template<typename R, typename... Args>
struct Delegate<R(Args...)> {
    explicit Delegate() = default;
    explicit Delegate(auto(*fn)(Args...) -> R) {
        bind(fn);
    }

    template<typename T, typename Self>
    explicit Delegate(T* self, auto(*fn)(Self*, Args...) -> R) {
        bind(self, fn);
    }

    inline void bind(auto(*fn)(Args...) -> R) {
        m_self = reinterpret_cast<void*>(fn);
        m_func = [](void* self, Args... args) {
            reinterpret_cast<decltype(fn)>(self)(args...);
        };
    }

    template<typename T, typename Self>
    inline void bind(T* self, auto(*fn)(Self*, Args...) -> R) {
        m_self = implicit_cast<Self*>(self);
        m_func = reinterpret_cast<decltype(m_func)>(fn);
    }

    template<typename... FwdArgs>
    inline auto operator()(FwdArgs&&... args) const -> R {
        return m_func(const_cast<void*>(m_self), std::forward<FwdArgs>(args)...);
    }
private:
    const void* m_self = nullptr;
    auto(*m_func)(void*, Args...) -> R = nullptr;
};

template<typename R, typename... Args>
Delegate(auto(*)(Args...) -> R) -> Delegate<R(Args...)>;

template<typename T, typename Self, typename R, typename... Args>
Delegate(T*, auto(*)(Self*, Args...) -> R) -> Delegate<R(Args...)>;