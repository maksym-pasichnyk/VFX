#pragma once

#include "Iter.hpp"

#include <vector>
#include <filesystem>

namespace cxx::filesystem {
    using path = std::filesystem::path;
    using directory_entry = std::filesystem::directory_entry;

    class __dir_element_proxy {
    public:
        inline auto operator*() -> directory_entry {
            return std::move(entry);
        }

    private:
        friend class directory_iterator;
        friend class recursive_directory_iterator;

        explicit __dir_element_proxy(directory_entry entry) : entry(std::move(entry)) {}
        __dir_element_proxy(__dir_element_proxy&& o) noexcept = default;

        directory_entry entry;
    };

    class __dir_stream;
    class directory_iterator {
    public:
        typedef directory_entry value_type;
        typedef ptrdiff_t difference_type;
        typedef value_type const* pointer;
        typedef value_type const& reference;
        typedef std::input_iterator_tag iterator_category;

    public:
        directory_iterator() noexcept;
        explicit directory_iterator(const path& __p);
        directory_iterator(const directory_iterator&) = default;
        directory_iterator(directory_iterator&&) = default;
        directory_iterator& operator=(const directory_iterator&) = default;
        directory_iterator& operator=(directory_iterator&& __o) noexcept {
            // non-default implementation provided to support self-move assign.
            if (this != &__o) {
                stream = _VSTD::move(__o.stream);
            }
            return *this;
        }
        ~directory_iterator() = default;

        auto operator*() const -> directory_entry {
            _LIBCPP_ASSERT(stream, "The end iterator cannot be dereferenced");
            return __dereference();
        }

        auto operator++() -> directory_iterator& {
            return __increment(nullptr);
        }

        auto operator++(int) -> __dir_element_proxy {
            __dir_element_proxy p(**this);
            __increment(nullptr);
            return p;
        }

        auto increment(std::error_code& ec) -> directory_iterator& {
            return __increment(&ec);
        }

    private:
        inline friend auto operator==(const directory_iterator& lhs, const directory_iterator& rhs) noexcept -> bool;

        auto __increment(std::error_code* ec) -> directory_iterator&;
        auto __dereference() const -> directory_entry;

    private:
        std::shared_ptr<__dir_stream> stream;
    };

    inline auto operator==(const directory_iterator& lhs, const directory_iterator& rhs) noexcept -> bool {
        return lhs.stream == rhs.stream;
    }

    inline auto operator!=(const directory_iterator& lhs, const directory_iterator& rhs) noexcept -> bool {
        return !(lhs == rhs);
    }

    inline auto begin(directory_iterator __iter) noexcept -> directory_iterator {
        return __iter;
    }

    inline auto end(directory_iterator) noexcept -> directory_iterator {
        return directory_iterator();
    }

    extern void init(const char* arg);
    extern void mount(const path& new_dir, const path& mount_point, bool append_to_path);

    extern auto read_file(const path& path) -> std::vector<char>;
}

template <>
inline constexpr bool ranges::enable_borrowed_range<cxx::filesystem::directory_iterator> = true;

template <>
inline constexpr bool ranges::enable_view<cxx::filesystem::directory_iterator> = true;
