#include "serializable.hpp"
#include <fstream>
#include <stdexcept>

void Serializable::save(std::ostream& out, Format format) const {
    if (format == Format::Binary) {
        save_binary(out);
    } else {
        save_text(out);
    }
}

void Serializable::load(std::istream& in, Format format) {
    if (format == Format::Binary) {
        load_binary(in);
    } else {
        load_text(in);
    }
}

void Serializable::save_file(const std::string& path, Format format) const {
    std::ofstream out(path, format == Format::Binary ? std::ios::binary : std::ios::out);
    if (!out) {
        throw std::runtime_error("cannot open " + path + " for writing");
    }
    save(out, format);
}

void Serializable::load_file(const std::string& path, Format format) {
    std::ifstream in(path, format == Format::Binary ? std::ios::binary : std::ios::in);
    if (!in) {
        throw std::runtime_error("cannot open " + path + " for reading");
    }
    load(in, format);
}
