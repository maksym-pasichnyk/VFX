//
// Created by Maksym Pasichnyk on 29.03.2023.
//

#pragma once

#include <atomic>
#include <typeinfo>

template<typename T>
class ManagedShared;

template<typename T>
auto TransferPtr(T* object_) -> ManagedShared<T>;

// TODO: remove template when Deducing this will be supported
template<typename T>
class ManagedObject {
protected:
    ManagedObject() : counter(1) {
//        fprintf(stdout, "Allocate(%s) %p\n", typeid(T).name(), this);
    }
    virtual ~ManagedObject() {
//        fprintf(stdout, "Deallocate(%s) %p\n", typeid(T).name(), this);
    }

public:
    auto retain() -> T* {
        counter += 1;
        return static_cast<T*>(this);
    }

    void release() {
        counter -= 1;
        if (counter == 0) {
            delete this;
        }
    }

    auto weak_from_this() -> ManagedShared<T> {
        return TransferPtr(this);
    }

    auto shared_from_this() -> ManagedShared<T> {
        return TransferPtr(retain());
    }

private:
    std::atomic_uint32_t counter = {};
};

template<typename T>
class ManagedShared {
//    template<typename U> friend auto RetainPtr(U* pointer) -> SharedPtr<U>;
    template<typename U> friend auto TransferPtr(U* object) -> ManagedShared<U>;

public:
    constexpr ManagedShared() noexcept = default;

    ManagedShared(const ManagedShared<T>& other) : object_(other.get()) {
        if (object_) {
            object_->ManagedObject::retain();
        }
    }

    template<typename U>
    ManagedShared(const ManagedShared<U>& other) : object_(other.get()) {
        if (object_) {
            object_->ManagedObject::retain();
        }
    }

    ManagedShared(ManagedShared<T>&& other) noexcept : object_(other.get()) {
        other.detach();
    }

    template<typename U>
    ManagedShared(ManagedShared<U>&& other) noexcept : object_(other.get()) {
        other.detach();
    }

    ~ManagedShared() {
        if (object_) {
            object_->ManagedObject::release();
        }
    }

public:
    constexpr auto get() const noexcept -> T* {
        return object_;
    }

    constexpr void detach() noexcept {
        object_ = nullptr;
    }

public:
    auto operator=(const ManagedShared<T>& other) -> ManagedShared<T>& {
        if (object_ != other.object_) {
            if (object_ != nullptr) {
                object_->ManagedObject::release();
            }
            object_ = other.get();
            if (object_ != nullptr) {
                object_->ManagedObject::retain();
            }
        }
        return *this;
    }

    template<typename U>
    auto operator=(const ManagedShared<U>& other) -> ManagedShared<T>& {
        if (object_ != other.get()) {
            if (object_ != nullptr) {
                object_->ManagedObject::release();
            }
            object_ = other.get();
            if (object_ != nullptr) {
                object_->ManagedObject::retain();
            }
        }
        return *this;
    }

    auto operator=(ManagedShared<T>&& other) -> ManagedShared<T>& {
        if (object_ != other.object_) {
            if (object_ != nullptr) {
                object_->ManagedObject::release();
            }
            object_ = other.get();
        }
        other.detach();
        return *this;
    }

    template<typename U>
    auto operator=(ManagedShared<U>&& other) -> ManagedShared<T>& {
        if (object_ != other.get()) {
            if (object_ != nullptr) {
                object_->ManagedObject::release();
            }
            object_ = other.get();
        }
        other.detach();
        return *this;
    }

    constexpr explicit operator bool() const noexcept {
        return object_ != nullptr;
    }

    constexpr auto operator->() const noexcept -> T* {
        return object_;
    }

    constexpr auto operator*() const noexcept -> T& {
        return *object_;
    }

    auto operator<=>(const ManagedShared<T>& other) const = default;

private:
    T* object_ = nullptr;
};

template<typename T>
inline auto TransferPtr(T* object) -> ManagedShared<T> {
    ManagedShared<T> shared = {};
    shared.object_ = object;
    return shared;
}

template<typename T, typename... Args>
inline auto MakeShared(Args&&... args) -> ManagedShared<T> {
    return TransferPtr(new T(std::forward<Args>(args)...));
}
