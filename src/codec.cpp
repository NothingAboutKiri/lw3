#include "codec.hpp"
#include <array>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include "array.hpp"

using namespace std;

namespace codec {

namespace {
constexpr std::array<char, 4> MAGIC = {'L', '3', 'B', '1'};
constexpr std::uint32_t MAX_STRING = 1U << 26U;
}

void write_u32(std::ostream& out, std::uint32_t value) {
    std::array<char, 4> bytes{};
    for (std::size_t i = 0; i < bytes.size(); i++) {
        bytes.at(i) = static_cast<char>((value >> (8 * i)) & 0xFFU);
    }
    out.write(bytes.data(), bytes.size());
}

std::uint32_t read_u32(std::istream& in) {
    std::array<char, 4> bytes{};
    if (!in.read(bytes.data(), bytes.size())) {
        throw std::runtime_error("unexpected end of binary data");
    }
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < bytes.size(); i++) {
        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(bytes.at(i))) << (8 * i);
    }
    return value;
}

void write_string(std::ostream& out, const std::string& value) {
    write_u32(out, static_cast<std::uint32_t>(value.size()));
    out.write(value.data(), static_cast<std::streamsize>(value.size()));
}

std::string read_string(std::istream& in) {
    const std::uint32_t length = read_u32(in);
    if (length > MAX_STRING) {
        throw std::runtime_error("string is too long");
    }
    std::string value(length, '\0');
    if (length > 0 && !in.read(value.data(), length)) {
        throw std::runtime_error("unexpected end of binary data");
    }
    return value;
}

void write_binary_header(std::ostream& out, const std::string& type, std::uint32_t count) {
    out.write(MAGIC.data(), MAGIC.size());
    write_string(out, type);
    write_u32(out, count);
}

std::uint32_t read_binary_header(std::istream& in, const std::string& type) {
    std::array<char, 4> magic{};
    if (!in.read(magic.data(), magic.size()) || magic != MAGIC) {
        throw std::runtime_error("not a lab3 binary file");
    }
    if (read_string(in) != type) {
        throw std::runtime_error("file contains another type, expected " + type);
    }
    return read_u32(in);
}

// ---------- Текстовый формат: JSON из ЛР №1 (file.cpp) ----------

namespace {

// Строка JSON в кавычках: " и \ экранируются, управляющие символы - через \u00XX
string JsonString(const string& text) {
    const char* hex = "0123456789abcdef";
    string result = "\"";
    for (const unsigned char c : text) {
        if (c == '"' || c == '\\') {
            result += '\\';
            result += (char)c;
        } else if (c < 0x20) {
            result += "\\u00";
            result += hex[c / 16];
            result += hex[c % 16];
        } else {
            result += (char)c;
        }
    }
    return result + "\"";
}

// Одна запись файла: "имя": ["x", "y", ...]
void WriteItem(ostream& file, bool& first, const string& name, const Array& values) {
    if (!first) {
        file << ",\n";
    }
    first = false;
    file << "  " << JsonString(name) << ": [";
    for (int i = 0; i < values.length(); i++) {
        if (i > 0) {
            file << ", ";
        }
        file << JsonString(values.at(i));
    }
    file << "]";
}

// Пропустить пробелы, табуляции и переводы строк
void SkipSpaces(const string& text, int& pos) {
    while (pos < (int)text.size() &&
           (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\n' || text[pos] == '\r')) {
        pos++;
    }
}

// Ожидается символ c (после пробелов)
bool Expect(const string& text, int& pos, char c) {
    SkipSpaces(text, pos);
    if (pos < (int)text.size() && text[pos] == c) {
        pos++;
        return true;
    }
    return false;
}

// Код символа \uXXXX -> байты UTF-8
void AddUtf8(string& out, int code) {
    if (code < 0x80) {
        out += (char)code;
    } else if (code < 0x800) {
        out += (char)(0xC0 | (code >> 6));
        out += (char)(0x80 | (code & 0x3F));
    } else {
        out += (char)(0xE0 | (code >> 12));
        out += (char)(0x80 | ((code >> 6) & 0x3F));
        out += (char)(0x80 | (code & 0x3F));
    }
}

// Строка в кавычках с escape-последовательностями
bool ReadString(const string& text, int& pos, string& out) {
    out = "";
    if (!Expect(text, pos, '"')) {
        return false;
    }
    while (pos < (int)text.size()) {
        const char c = text[pos++];
        if (c == '"') {
            return true;
        }
        if (c != '\\') {
            out += c;
            continue;
        }
        if (pos >= (int)text.size()) {
            return false;
        }
        const char e = text[pos++];
        switch (e) {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'n': out += '\n'; break;
            case 't': out += '\t'; break;
            case 'r': out += '\r'; break;
            case 'b': out += '\b'; break;
            case 'f': out += '\f'; break;
            case 'u': {
                if (pos + 4 > (int)text.size()) {
                    return false;
                }
                int code = 0;
                for (int i = 0; i < 4; i++) {
                    const char h = text[pos++];
                    code *= 16;
                    if (h >= '0' && h <= '9') code += h - '0';
                    else if (h >= 'a' && h <= 'f') code += h - 'a' + 10;
                    else if (h >= 'A' && h <= 'F') code += h - 'A' + 10;
                    else return false;
                }
                if (code >= 0xD800 && code <= 0xDFFF) {
                    return false;   // половинки суррогатных пар не поддерживаются
                }
                AddUtf8(out, code);
                break;
            }
            default:
                return false;
        }
    }
    return false;   // нет закрывающей кавычки
}

// Значение в массиве: строка в кавычках или число без кавычек (например, 15)
bool ReadValue(const string& text, int& pos, string& out) {
    SkipSpaces(text, pos);
    if (pos < (int)text.size() && text[pos] == '"') {
        return ReadString(text, pos, out);
    }
    out = "";
    while (pos < (int)text.size() && text[pos] != ',' && text[pos] != ']' &&
           text[pos] != ' ' && text[pos] != '\n' && text[pos] != '\r' && text[pos] != '\t') {
        out += text[pos++];
    }
    return !out.empty();
}

// Разобрать весь файл; при ошибке возвращает false и ничего не добавляет
bool ParseJson(const string& text, map<string, Array>& result) {
    int pos = 0;
    if (!Expect(text, pos, '{')) {
        return false;
    }
    if (Expect(text, pos, '}')) {
        SkipSpaces(text, pos);
        return pos == (int)text.size();
    }
    while (true) {
        string name;
        if (!ReadString(text, pos, name) || !Expect(text, pos, ':') || !Expect(text, pos, '[')) {
            return false;
        }
        Array values;
        if (!Expect(text, pos, ']')) {
            while (true) {
                string value;
                if (!ReadValue(text, pos, value)) {
                    return false;
                }
                values.push_back(value);
                if (Expect(text, pos, ']')) {
                    break;
                }
                if (!Expect(text, pos, ',')) {
                    return false;
                }
            }
        }
        result[name] = values;
        if (Expect(text, pos, '}')) {
            break;
        }
        if (!Expect(text, pos, ',')) {
            return false;
        }
    }
    SkipSpaces(text, pos);
    return pos == (int)text.size();
}

}  // namespace

void write_json(ostream& out, const string& type, const Array& values) {
    out << "{\n";
    bool first = true;
    WriteItem(out, first, type, values);
    out << "\n}\n";
}

Array read_json(istream& in, const string& type) {
    stringstream buffer;
    buffer << in.rdbuf();
    map<string, Array> result;
    if (!ParseJson(buffer.str(), result)) {
        throw runtime_error("not a lab3 text (JSON) file");
    }
    auto it = result.find(type);
    if (result.size() != 1 || it == result.end()) {
        throw runtime_error("file contains another type, expected " + type);
    }
    return it->second;
}

}
