#include "filesystem.hpp"
#include "physfs.h"
#include "fstream.hpp"

class cxx::filesystem::__dir_stream {
public:
    cxx::filesystem::path dir;
    char** raw;
    size_t i = 0;

    explicit __dir_stream(const cxx::filesystem::path& dir) : dir(dir) {
        raw = PHYSFS_enumerateFiles(dir.c_str());
    }

    ~__dir_stream() {
        PHYSFS_freeList(raw);
    }

    auto operator*() const -> cxx::filesystem::directory_entry {
        return cxx::filesystem::directory_entry(dir / raw[i]);
    }
};

cxx::filesystem::directory_iterator::directory_iterator() noexcept : stream(nullptr) {}
cxx::filesystem::directory_iterator::directory_iterator(const cxx::filesystem::path& dir) : stream(std::make_shared<__dir_stream>(dir)) {}

auto cxx::filesystem::directory_iterator::__increment(std::error_code* ec) -> directory_iterator& {
    stream->i++;
    if (stream->raw[stream->i] == nullptr) {
        stream = nullptr;
    }
    return *this;
}

auto cxx::filesystem::directory_iterator::__dereference() const -> directory_entry {
    return **stream;
}

void cxx::filesystem::init(const char* arg) {
    PHYSFS_init(arg);
}

void cxx::filesystem::mount(const path& new_dir, const filesystem::path& mount_point, bool append_to_path) {
    PHYSFS_mount(new_dir.c_str(), mount_point.c_str(), append_to_path);
}

auto cxx::filesystem::read_file(const cxx::filesystem::path& path) -> std::vector<char> {
    cxx::ifstream file{path};
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}