#pragma once

#include <cstdint>
#include <memory>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

//static_assert(sizeof(u64) == sizeof(size_t));

template <typename T>
struct Arc final : std::shared_ptr<T> {
    using std::shared_ptr<T>::shared_ptr;

    template<std::derived_from<T> U>
    Arc(std::shared_ptr<U> other) : std::shared_ptr<T>(std::move(other)) {}

    static auto alloc(auto&&... args) -> Arc {
        return Arc{std::make_shared<T>(std::forward<decltype(args)>(args)...)};
    }
};

template <typename T>
Arc(std::shared_ptr<T>) -> Arc<T>;

template <typename T>
struct Box final : std::unique_ptr<T> {
    using std::unique_ptr<T>::unique_ptr;

    template<std::derived_from<T> U>
    Box(std::unique_ptr<U>&& other) : std::unique_ptr<T>(std::move(other)) {}

    static auto alloc(auto&&... args) -> Box {
        return Box(std::make_unique<T>(std::forward<decltype(args)>(args)...));
    }
};

template <typename T>
Box(std::unique_ptr<T>) -> Box<T>;