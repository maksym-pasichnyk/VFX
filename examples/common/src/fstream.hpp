#pragma once

#include <array>
#include "filesystem.hpp"

namespace cxx {
    struct File {
        void* opaque;
    };

    class filebuf : public std::streambuf {
    public:
        filebuf();
        ~filebuf() override;

    public:
        auto open(const std::string& path) -> bool;
        [[nodiscard]] auto is_open() const -> bool;
        [[nodiscard]] auto underflow() -> int_type override;

        // todo: remove?
        [[nodiscard]] auto length() const -> size_t;

    private:
        File* file;
        std::array<char, 1024> tmp;
    };

    class ifstream : public std::istream {
    public:
        explicit ifstream(const std::string& path);

        // todo: remove?
        [[nodiscard]] auto length() const -> size_t;

    private:
        filebuf buf;
    };
}