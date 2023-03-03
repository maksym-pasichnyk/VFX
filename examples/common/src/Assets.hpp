#pragma once

#include <vector>
#include <fstream>
#include <filesystem>

struct Assets {
    static auto readFile(const std::filesystem::path& path) -> std::vector<char> {
        std::ifstream file{path};
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
};