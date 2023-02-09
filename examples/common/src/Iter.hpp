#pragma once

#include "range/v3/all.hpp"

namespace cxx {
    template<typename T>
    struct Iter : T {
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

        template<typename Fn>
        constexpr auto any(Fn&& fn) {
            return ranges::any_of(*this, wrap(std::forward<Fn>(fn)));
        }

        template<typename Fn>
        constexpr auto all(Fn&& fn) {
            return ranges::all_of(*this, wrap(std::forward<Fn>(fn)));
        }

        template<typename Fn>
        constexpr auto where(Fn&& fn) {
            return ::cxx::Iter{ranges::views::filter(*this, wrap(std::forward<Fn>(fn)))};
        }

        template<typename Fn>
        constexpr auto map(Fn&& fn) {
            return ::cxx::Iter{ranges::views::transform(*this, wrap(std::forward<Fn>(fn)))};
        }

        template<typename Fn>
        constexpr auto flatmap(Fn&& fn) {
            return map(std::forward<Fn>(fn)).join();
        }

        template<typename R, typename Fn>
        constexpr auto reduce(R&& init, Fn&& fn) -> R {
            return ranges::accumulate(*this, init, wrap(std::forward<Fn>(fn)));
        }

        template<typename Fn>
        constexpr void for_each(Fn&& fn) {
            ranges::for_each(*this, wrap(std::forward<Fn>(fn)));
        }

        constexpr auto zip(auto&& ... iter) {
            return ::cxx::Iter{ranges::views::zip(*this, std::forward<decltype(iter)>(iter)...)};
        }

        constexpr auto join() {
            return ::cxx::Iter{ranges::views::join(*this)};
        }

        template<typename Tp>
        constexpr auto to() -> Tp {
            return ranges::to<Tp>(*this);
        }

        template<typename U>
        constexpr auto split(U&& val) {
            return ::cxx::Iter{ranges::views::split(*this, std::forward<U>(val))};
        }

        constexpr auto collect() {
            return ranges::to_vector(*this);
        }
    };

    template<typename T>
    Iter(T) -> Iter<T>;

    template<typename T>
    inline auto iter(T& range) {
        return Iter{ranges::views::ref(range)};
    }

    template<typename T>
    inline auto iter(T&& range) {
        return Iter{ranges::make_view_closure(std::forward<T>(range))};
    }
}