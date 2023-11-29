#pragma once

#include "JsonElement.hpp"

#include <istream>

struct JsonParser {
public:
    static auto fromStream(std::istream& stream) -> JsonElement;
    static auto fromStream(std::istream&& stream) -> JsonElement;

private:
    static auto next(std::istream& stream) -> JsonElement;
    static auto array(std::istream& stream) -> JsonElement;
    static auto object(std::istream& stream) -> JsonElement;
    static auto string(std::istream& stream) -> std::string;
    static auto number(std::istream& stream) -> JsonElement;
    static auto boolean(std::istream& stream) -> JsonElement;

    static void trim(std::istream& stream);
    static auto check(std::istream& stream, char ch) -> bool;
    static void expect(std::istream& stream, char ch, std::string const& msg);
};
