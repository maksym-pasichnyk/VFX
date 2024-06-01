//
// Created by Maksym Pasichnyk on 29.03.2023.
//

#pragma once

#include <atomic>
#include <memory>
#include <typeinfo>

template<typename T>
class rc;

class ManagedObject {
protected:
    explicit ManagedObject() : __strong_references(1) {}
    virtual ~ManagedObject() = default;

public:
    template<typename Self>
    auto retain(this Self& self) -> Self* {
        self.__strong_references += 1;
        return std::addressof(self);
    }

    template<typename Self>
    void release(this Self& self) {
        if (self.__strong_references.fetch_sub(1) == 1) {
            delete std::addressof(self);
        }
    }

    template<typename Self>
    auto shared_from_this(this Self& self) -> rc<Self> {
        return rc(self.retain());
    }

private:
    std::atomic_uint64_t __strong_references = {};
};

template<typename T>
class rc final {
public:
    template<typename U>
    friend class rc;

    constexpr rc() noexcept : __handle(nullptr) {}
    constexpr explicit rc(T* object) noexcept : __handle(object) {}

    rc(rc<T> const& other) : __handle(other.get()) {
        if (__handle) {
            __handle->ManagedObject::retain();
        }
    }

    template<typename U>
    rc(rc<U> const& other) : __handle(other.get()) {
        if (__handle) {
            __handle->ManagedObject::retain();
        }
    }

    rc(rc<T>&& other) noexcept = default;

    template<typename U>
    rc(rc<U>&& other) noexcept : __handle(other.detach()) {}

public:
    constexpr auto get(this rc const& self) noexcept -> T* {
        return self.__handle.get();
    }

    constexpr auto detach(this rc& self) noexcept -> T* {
        return self.__handle.release();
    }

public:
    auto operator=(this rc& self, rc<T> const& other) -> rc<T>& {
        self = auto(other);
        return self;
    }

//    template<typename U>
//    auto operator=(rc<U> const& other) -> rc<T>& {
//        *this = auto(other);
//        return *this;
//    }

    auto operator=(this rc& self, rc<T>&& other) -> rc<T>& {
        self.__handle.swap(other.__handle);
        return self;
    }

//    template<typename U>
//    auto operator=(rc<U>&& other) -> rc<T>& {
//        __handle.swap(other.__handle);
//        return *this;
//    }

    constexpr explicit operator bool(this rc const& self) noexcept {
        return self.__handle.get() != nullptr;
    }

    constexpr auto operator->(this rc const& self) noexcept -> T* {
        return self.__handle.get();
    }

    constexpr auto operator*(this rc const& self) noexcept -> T& {
        return *self.__handle.get();
    }

    friend auto operator<=>(rc<T> const&, rc<T> const&) noexcept = default;

public:
    template<typename... Args>
    static auto init(Args&&... args) {
        return rc(new T(std::forward<Args>(args)...));
    }

private:
    struct __deleter {
        static void operator()(T* ptr) {
            ptr->ManagedObject::release();
        }
    };

    std::unique_ptr<T, __deleter> __handle;
};