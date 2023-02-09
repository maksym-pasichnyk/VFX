#pragma once

#include <vector>
#include "fstream.hpp"
#include "filesystem.hpp"

struct Assets {
    static auto readFile(const cxx::filesystem::path& path) -> std::vector<char> {
        cxx::ifstream file{path};
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
};