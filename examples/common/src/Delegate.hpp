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
        mObject = reinterpret_cast<void*>(fn);
        mFunction = [](void* self, Args... args) {
            reinterpret_cast<decltype(fn)>(self)(args...);
        };
    }

    template<typename T, typename Self>
    inline void bind(T* self, auto(*fn)(Self*, Args...) -> R) {
        mObject = implicit_cast<Self*>(self);
        mFunction = reinterpret_cast<decltype(mFunction)>(fn);
    }

    template<typename... FwdArgs>
    inline auto operator()(FwdArgs&&... args) const -> R {
        return mFunction(const_cast<void*>(mObject), std::forward<FwdArgs>(args)...);
    }
private:
    void* mObject = nullptr;
    auto(*mFunction)(void*, Args...) -> R = nullptr;
};

template<typename R, typename... Args>
Delegate(auto(*)(Args...) -> R) -> Delegate<R(Args...)>;

template<typename T, typename Self, typename R, typename... Args>
Delegate(T*, auto(*)(Self*, Args...) -> R) -> Delegate<R(Args...)>;