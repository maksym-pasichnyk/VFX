//
// Created by Maksym Pasichnyk on 29.03.2023.
//

#pragma once

#include <atomic>
#include <typeinfo>

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

private:
    std::atomic_uint32_t counter = {};
};

template<typename T>
class ManagedShared {
//    template<typename U> friend auto RetainPtr(U* pointer) -> SharedPtr<U>;
    template<typename U> friend auto MakeShared(U* pointer) -> ManagedShared<U>;

public:
    constexpr ManagedShared() noexcept = default;

    ManagedShared(const ManagedShared<T>& other) : stored(other.stored) {
        if (stored) {
            stored->ManagedObject::retain();
        }
    }

    template<typename U>
    ManagedShared(const ManagedShared<U>& other) : stored(other.get()) {
        if (stored) {
            stored->ManagedObject::retain();
        }
    }

    ManagedShared(ManagedShared<T>&& other) noexcept : stored(other.stored) {
        other.stored = nullptr;
    }

    template<typename U>
    ManagedShared(ManagedShared<U>&& other) noexcept : stored(other.get()) {
        other.detach();
    }

    ~ManagedShared() {
        if (stored) {
            stored->ManagedObject::release();
        }
    }

public:
    constexpr auto get() const noexcept -> T* {
        return stored;
    }

    constexpr void detach() noexcept {
        stored = nullptr;
    }

public:
    auto operator=(const ManagedShared<T>& other) -> ManagedShared<T>& {
        if (stored != other.stored) {
            if (stored != nullptr) {
                stored->ManagedObject::release();
            }
            stored = other.get();
            if (stored != nullptr) {
                stored->ManagedObject::retain();
            }
        }
        return *this;
    }

    template<typename U>
    auto operator=(const ManagedShared<U>& other) -> ManagedShared<T>& {
        if (stored != other.get()) {
            if (stored != nullptr) {
                stored->ManagedObject::release();
            }
            stored = other.get();
            if (stored != nullptr) {
                stored->ManagedObject::retain();
            }
        }
        return *this;
    }

    auto operator=(ManagedShared<T>&& other) -> ManagedShared<T>& {
        if (stored != other.stored) {
            if (stored != nullptr) {
                stored->ManagedObject::release();
            }
            stored = other.get();
        }
        other.detach();
        return *this;
    }

    template<typename U>
    auto operator=(ManagedShared<U>&& other) -> ManagedShared<T>& {
        if (stored != other.get()) {
            if (stored != nullptr) {
                stored->ManagedObject::release();
            }
            stored = other.get();
        }
        other.detach();
        return *this;
    }

    constexpr explicit operator bool() const noexcept {
        return stored != nullptr;
    }

    constexpr auto operator->() const noexcept -> T* {
        return stored;
    }

    constexpr auto operator*() const noexcept -> T& {
        return *stored;
    }

    auto operator<=>(const ManagedShared<T>& other) const = default;

private:
    T* stored = nullptr;
};

template<typename T>
inline auto MakeShared(T* stored) -> ManagedShared<T> {
    ManagedShared<T> shared = {};
    shared.stored = stored;
    return shared;
}

