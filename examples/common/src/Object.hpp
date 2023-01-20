//
// Created by Maksym Pasichnyk on 09.02.2023.
//

#pragma once

#include <atomic>
#include <typeinfo>

class Object {
public:
    Object() : counter(1) {
//            fprintf(stdout, "Allocate %p\n", this);
    }
    virtual ~Object() {
//            fprintf(stdout, "Deallocate %p\n", this);
    }

    void retain() {
        counter += 1;
    }

    void release() {
        counter -= 1;
        if (counter == 0) {
            delete this;
        }
    }

    auto retainCount() const -> uint32_t {
        return counter.load();
    }

private:
    std::atomic_uint32_t counter = {};
};

template<typename T>
class SharedPtr {
    template<typename U> friend auto RetainPtr(U* pointer) -> SharedPtr<U>;
    template<typename U> friend auto TransferPtr(U* pointer) -> SharedPtr<U>;

public:
    constexpr SharedPtr() noexcept = default;

    SharedPtr(const SharedPtr<T>& other) : pObject(other.pObject) {
        if (pObject) {
            pObject->Object::retain();
        }
    }

    template<typename U>
    SharedPtr(const SharedPtr<U>& other) : pObject(other.get()) {
        if (pObject) {
            pObject->Object::retain();
        }
    }

    SharedPtr(SharedPtr<T>&& other) noexcept : pObject(other.pObject) {
        other.pObject = nullptr;
    }

    template<typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept : pObject(other.get()) {
        other.detach();
    }

    ~SharedPtr() {
        if (pObject) {
            pObject->Object::release();
        }
    }

public:
    constexpr auto get() const noexcept -> T* {
        return pObject;
    }

    constexpr void detach() noexcept {
        pObject = nullptr;
    }

public:
    auto operator=(const SharedPtr<T>& other) -> SharedPtr<T>& {
        if (pObject != other.pObject) {
            if (pObject != nullptr) {
                pObject->Object::release();
            }
            pObject = other.get();
            if (pObject != nullptr) {
                pObject->Object::retain();
            }
        }
        return *this;
    }

    template<typename U>
    auto operator=(const SharedPtr<U>& other) -> SharedPtr<T>& {
        if (pObject != other.get()) {
            if (pObject != nullptr) {
                pObject->Object::release();
            }
            pObject = other.get();
            if (pObject != nullptr) {
                pObject->Object::retain();
            }
        }
        return *this;
    }

    auto operator=(SharedPtr<T>&& other) -> SharedPtr<T>& {
        if (pObject != other.pObject) {
            if (pObject != nullptr) {
                pObject->Object::release();
            }
            pObject = other.get();
        }
        other.detach();
        return *this;
    }

    template<typename U>
    auto operator=(SharedPtr<U>&& other) -> SharedPtr<T>& {
        if (pObject != other.get()) {
            if (pObject != nullptr) {
                pObject->Object::release();
            }
            pObject = other.get();
        }
        other.detach();
        return *this;
    }

    constexpr explicit operator bool() const noexcept {
        return pObject != nullptr;
    }

    constexpr auto operator->() const noexcept -> T* {
        return pObject;
    }

    constexpr auto operator*() const noexcept -> T& {
        return *pObject;
    }

    auto operator<=>(const SharedPtr<T>& other) const = default;

public:
    static auto of(auto&&... args) -> SharedPtr<T> {
        return TransferPtr(new T(std::forward<decltype(args)>(args)...));
    }

private:
    T* pObject = nullptr;
};

template<typename T>
using sp = SharedPtr<T>;

template<typename T>
inline auto TransferPtr(T* pObject) -> SharedPtr<T> {
    SharedPtr<T> shared = {};
    shared.pObject = pObject;
    return shared;
}

template<typename T>
inline auto RetainPtr(T* pObject) -> SharedPtr<T> {
    pObject->Object::retain();
    return TransferPtr(pObject);
}
