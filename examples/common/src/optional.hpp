//
// Created by Maksym Pasichnyk on 10.02.2023.
//

#include <optional>

namespace cxx {
    template<typename T>
    struct optional : std::optional<T> {
        using std::optional<T>::optional;

        auto map(auto&& fn) -> optional<std::invoke_result_t<decltype(fn), T>> {
            if (this->has_value()) {
                return fn(*this);
            } else {
                return std::nullopt;
            }
        }

        auto map(auto&& fn) const -> optional<std::invoke_result_t<decltype(fn), T>> {
            if (this->has_value()) {
                return fn(*this);
            } else {
                return std::nullopt;
            }
        }
    };

    template<typename T>
    optional(T) -> optional<T>;
}