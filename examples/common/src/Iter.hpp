#pragma once

#include "range/v3/all.hpp"

namespace cxx {
    template<typename T>
    struct Iter {
        T impl;

        template<typename Fn>
        static constexpr auto wrap(Fn&& fn) {
            return [fn = std::forward<Fn>(fn)](auto&& args) {
                if constexpr (std::is_invocable_v < Fn, decltype(args) >) {
                    return fn(std::forward<decltype(args)>(args));
                } else {
                    return std::apply(fn, std::forward<decltype(args)>(args));
                }
            };
        }

        friend constexpr auto begin(const Iter& iter) noexcept {
            return std::ranges::begin(iter.impl);
        }

        friend constexpr auto end(const Iter& iter) noexcept {
            return std::ranges::end(iter.impl);
        }

        template<typename Fn>
        constexpr auto any(Fn&& fn) {
            return ranges::any_of(impl, wrap(std::forward<Fn>(fn)));
        }

        template<typename Fn>
        constexpr auto all(Fn&& fn) {
            return ranges::all_of(impl, wrap(std::forward<Fn>(fn)));
        }

        template<typename Fn>
        constexpr auto where(Fn&& fn) {
            return ::cxx::Iter{ranges::views::filter(impl, wrap(std::forward<Fn>(fn)))};
        }

        template<typename Fn>
        constexpr auto map(Fn&& fn) {
            return ::cxx::Iter{ranges::views::transform(impl, wrap(std::forward<Fn>(fn)))};
        }

        template<typename Fn>
        constexpr auto flatmap(Fn&& fn) {
            return map(std::forward<Fn>(fn)).join();
        }

        template<typename R, typename Fn>
        constexpr auto reduce(R&& init, Fn&& fn) -> R {
            return ranges::accumulate(impl, init, wrap(std::forward<Fn>(fn)));
        }

        template<typename Fn>
        constexpr void for_each(Fn&& fn) {
            ranges::for_each(impl, wrap(std::forward<Fn>(fn)));
        }

        constexpr auto zip(auto&& ... iter) {
            return ::cxx::Iter{ranges::views::zip(impl, std::forward<decltype(iter)>(iter)...)};
        }

        constexpr auto join() {
            return ::cxx::Iter{ranges::views::join(impl)};
        }

        template<typename Tp>
        constexpr auto to() -> Tp {
            return ranges::to<Tp>(impl);
        }

        template<typename U>
        constexpr auto split(U&& val) {
            return ::cxx::Iter{ranges::views::split(impl, std::forward<U>(val))};
        }

        constexpr auto collect() {
            return ranges::to_vector(impl);
        }
    };

    template<typename T>
    Iter(T) -> Iter<T>;

    template<typename T>
    inline auto iter(T& range) {
        return Iter<T&>{range};
    }

    template<typename T>
    inline auto iter(T&& range) {
        return Iter<T>{std::forward<T>(range)};
    }
}