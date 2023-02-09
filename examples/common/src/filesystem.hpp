#pragma once

#include <vector>
#include <filesystem>

namespace cxx::filesystem {
    using path = std::filesystem::path;

    extern void init(const char* arg);
    extern void mount(const path& new_dir, const path& mount_point, bool append_to_path);

    // todo: remove
    extern void iterate_recursive(const path& dir, const std::function<void(path)>& fn);

    extern auto read_file(const path& path) -> std::vector<char>;
}