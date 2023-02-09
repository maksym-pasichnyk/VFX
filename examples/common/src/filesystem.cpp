#include "filesystem.hpp"
#include "physfs.h"
#include "fstream.hpp"

void cxx::filesystem::init(const char* arg) {
    PHYSFS_init(arg);
}

void cxx::filesystem::mount(const path& new_dir, const filesystem::path& mount_point, bool append_to_path) {
    PHYSFS_mount(new_dir.c_str(), mount_point.c_str(), append_to_path);
}

void cxx::filesystem::iterate_recursive(const path& dir, const std::function<void(path)>& fn) {
    auto files = PHYSFS_enumerateFiles(dir.c_str());
    for (auto file = files; *file != nullptr; file++) {
        auto path = dir / *file;

        if (PHYSFS_isDirectory(path.c_str())) {
            iterate_recursive(path, fn);
        } else {
            fn(path);
        }
    }
    PHYSFS_freeList(files);
}
auto cxx::filesystem::read_file(const cxx::filesystem::path& path) -> std::vector<char> {
    cxx::ifstream file{path};
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
