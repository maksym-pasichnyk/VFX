#include "fstream.hpp"
#include "physfs.h"

cxx::filebuf::filebuf() : file(nullptr), tmp() {}
cxx::filebuf::~filebuf() {
    if (file != nullptr) {
        PHYSFS_close(reinterpret_cast<PHYSFS_File*>(file));
        file = nullptr;
    }
}

auto cxx::filebuf::open(const std::string& path) -> bool {
    if (file != nullptr) { return false; }
    file = reinterpret_cast<File*>(PHYSFS_openRead(path.c_str()));
    return file != nullptr;
}

auto cxx::filebuf::is_open() const -> bool {
    return file != nullptr;
}

auto cxx::filebuf::underflow() -> int {
    if (PHYSFS_eof(reinterpret_cast<PHYSFS_File*>(file))) {
        return traits_type::eof();
    }
    const auto len = PHYSFS_readBytes(reinterpret_cast<PHYSFS_File*>(file), tmp.data(), tmp.size());
    if (len < 1) {
        return traits_type::eof();
    }
    setg(tmp.data(), tmp.data(), tmp.data() + len);
    return traits_type::to_int_type(*gptr());
}

auto cxx::filebuf::length() const -> size_t {
    return PHYSFS_fileLength(reinterpret_cast<PHYSFS_File*>(file));
}

cxx::ifstream::ifstream(const std::string& path) : std::istream(&buf) {
    if (!buf.open(path)) {
        setstate(ios_base::failbit);
    }
}

auto cxx::ifstream::length() const -> size_t {
    return buf.length();
}