#include <istream>
#include <ostream>
#include "array.hpp"
#include "codec.hpp"
#include "container.hpp"

void Sequence::save_binary(std::ostream& out) const {
    const Array items = values();
    codec::write_binary_header(out, type_name(), static_cast<std::uint32_t>(items.length()));
    for (int i = 0; i < items.length(); i++) {
        codec::write_string(out, items.at(i));
    }
}

void Sequence::load_binary(std::istream& in) {
    const std::uint32_t count = codec::read_binary_header(in, type_name());
    Array items;
    for (std::uint32_t i = 0; i < count; i++) {
        items.push_back(codec::read_string(in));
    }
    clear();
    for (int i = 0; i < items.length(); i++) {
        restore(items.at(i));
    }
}

void Sequence::save_text(std::ostream& out) const {
    codec::write_json(out, type_name(), values());
}

void Sequence::load_text(std::istream& in) {
    const Array items = codec::read_json(in, type_name());
    clear();
    for (int i = 0; i < items.length(); i++) {
        restore(items.at(i));
    }
}
