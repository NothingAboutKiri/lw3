#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>

enum class Format : std::uint8_t { Binary, Text };

class Serializable {
public:
    Serializable() = default;
    Serializable(const Serializable&) = default;
    Serializable(Serializable&&) = default;
    Serializable& operator=(const Serializable&) = default;
    Serializable& operator=(Serializable&&) = default;
    virtual ~Serializable() = default;

    virtual void save_binary(std::ostream& out) const = 0;
    virtual void load_binary(std::istream& in) = 0;
    virtual void save_text(std::ostream& out) const = 0;
    virtual void load_text(std::istream& in) = 0;

    void save(std::ostream& out, Format format) const;
    void load(std::istream& in, Format format);
    void save_file(const std::string& path, Format format) const;
    void load_file(const std::string& path, Format format);
};
