#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>

class Array;

namespace codec {

void write_u32(std::ostream& out, std::uint32_t value);
std::uint32_t read_u32(std::istream& in);
void write_string(std::ostream& out, const std::string& value);
std::string read_string(std::istream& in);

void write_binary_header(std::ostream& out, const std::string& type, std::uint32_t count);
std::uint32_t read_binary_header(std::istream& in, const std::string& type);

void write_json(std::ostream& out, const std::string& type, const Array& values);
Array read_json(std::istream& in, const std::string& type);

}
