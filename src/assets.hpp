#pragma once

#include <vector>
#include <fstream>
#include <filesystem>

struct Assets {
    static auto readFile(const std::filesystem::path& path) -> std::vector<char> {
        auto full_path = std::filesystem::current_path() / "assets" / path;

        std::vector<char> out(std::filesystem::file_size(full_path));
        std::ifstream file{full_path, std::ios::binary};
        file.read(out.data(), std::streamsize(out.size()));
        return out;
    }
};