#include "JsonParser.hpp"

auto JsonParser::fromStream(std::istream& stream) -> JsonElement {
    return next(stream);
}

auto JsonParser::fromStream(std::istream&& stream) -> JsonElement {
    return next(stream);
}

auto JsonParser::next(std::istream& stream) -> JsonElement {
    while (!stream.eof()) {
        char ch = char(stream.peek());

        if (isspace(ch)) {
            stream.get();
            continue;
        }

        if (ch == '{') {
            return object(stream);
        }
        if (ch == '[') {
            return array(stream);
        }
        if (ch == '"') {
            return string(stream);
        }
        if (ch == '-' || ch == '+' || isdigit(ch)) {
            return number(stream);
        }
        if (isalpha(ch)) {
            return boolean(stream);
        }
        throw std::runtime_error("unexpected character");
    }
    throw std::runtime_error("unexpected end");
}

auto JsonParser::array(std::istream& stream) -> JsonElement {
    JsonArray out = {};
    stream.get();
    if (!check(stream, ']')) {
        while (true) {
            trim(stream);
            out.emplace_back(next(stream));
            trim(stream);
            if (check(stream, ']')) {
                break;
            }
            trim(stream);
            expect(stream, ',', "unexpected character");
            stream.get();
        }
    }
    stream.get();
    return out;
}

auto JsonParser::object(std::istream& stream) -> JsonElement {
    JsonObject out = {};
    stream.get();
    trim(stream);
    if (!check(stream, '}')) {
        while (true) {
            trim(stream);
            expect(stream, '"', "unexpected character");
            auto key = string(stream);
            trim(stream);
            expect(stream, ':', "unexpected character");
            stream.get();
            auto value = next(stream);

            out.emplace(std::move(key), std::move(value));

            trim(stream);
            if (check(stream, '}')) {
                break;
            }
            trim(stream);
            expect(stream, ',', "unexpected character");
            stream.get();
        }
    }
    stream.get();
    return out;
}

auto JsonParser::string(std::istream& stream) -> std::string {
    std::string tmp = {};
    stream.get();
    while (!stream.eof()) {
        char ch = char(stream.peek());

        if (ch == '"') {
            stream.get();
            return tmp;
        }
        if (ch == '\n') {
            break;
        }
        // todo: escaped characters
        tmp.push_back(ch);
        stream.get();
    }
    throw std::runtime_error("unterminated string");
}

auto JsonParser::number(std::istream& stream) -> JsonElement {
    std::string tmp = {};
    tmp.push_back(char(stream.peek()));
    stream.get();

    bool decimal = true;
    while (!stream.eof()) {
        char ch = char(stream.peek());
        if (isdigit(ch) || (ch == '.' && std::exchange(decimal, false))) {
            tmp.push_back(ch);
            stream.get();
        } else {
            break;
        }
    }
    if (decimal) {
        return std::stoll(tmp);
    } else {
        return std::stod(tmp);
    }
}

auto JsonParser::boolean(std::istream& stream) -> JsonElement {
    std::string tmp = {};
    while (!stream.eof() && isalnum(char(stream.peek()))) {
        tmp.push_back(char(stream.peek()));
        stream.get();
    }
    if (tmp == "true") {
        return true;
    }
    if (tmp == "false") {
        return false;
    }
    throw std::runtime_error("unexpected value");
}

void JsonParser::trim(std::istream& stream) {
    while (!stream.eof() && isspace(char(stream.peek()))) {
        stream.get();
    }
}

auto JsonParser::check(std::istream& stream, char ch) -> bool {
    return !stream.eof() && char(stream.peek()) == ch;
}

void JsonParser::expect(std::istream& stream, char ch, const std::string& msg) {
    if (!check(stream, ch)) {
        throw std::runtime_error(msg);
    }
}
