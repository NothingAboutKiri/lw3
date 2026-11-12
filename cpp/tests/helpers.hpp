#pragma once
#include <algorithm>
#include <functional>
#include <ostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "array.hpp"
#include "doubly_list.hpp"
#include "hash_table.hpp"
#include "queue.hpp"
#include "rb_tree.hpp"
#include "singly_list.hpp"
#include "stack.hpp"

inline std::string join(const Array& a) {
    std::string out;
    for (int i = 0; i < a.length(); i++) {
        out += (i == 0 ? "" : " ") + a.at(i);
    }
    return out;
}

inline std::string join(const Sequence& s) {
    return join(s.values());
}

inline std::string pairs(const HashTable& t) {
    std::vector<std::string> items;
    t.for_each_pair([&items](const std::string& k, const std::string& v) { items.push_back(k + "=" + v); });
    std::sort(items.begin(), items.end());
    std::string out;
    for (const auto& s : items) {
        out += (out.empty() ? "" : " ") + s;
    }
    return out;
}

// lab1-структуры не копируются, поэтому результат загружается в переданный объект
template <typename T>
void roundtrip(const T& source, T& target, Format format) {
    std::stringstream buffer;
    source.save(buffer, format);
    target.load(buffer, format);
}

inline std::string text_of(const Serializable& s) {
    std::stringstream out;
    s.save(out, Format::Text);
    return out.str();
}

// ---------- Параметры для однотипных проверок ----------
using Maker = std::function<std::unique_ptr<Container>()>;

// Образец: как создать пустой контейнер и контейнер с данными
struct Sample {
    std::string name;
    Maker make_empty;
    Maker make_filled;
};

inline std::ostream& operator<<(std::ostream& out, const Sample& s) {
    return out << s.name;
}

// Образец + формат файла
struct FormatSample {
    Sample sample;
    Format format;
};

inline std::ostream& operator<<(std::ostream& out, const FormatSample& s) {
    return out << s.sample.name << (s.format == Format::Binary ? "/binary" : "/text");
}

// Вид хеш-таблицы
struct TableKind {
    std::string name;
    std::function<std::unique_ptr<HashTable>()> make;
};

inline std::ostream& operator<<(std::ostream& out, const TableKind& k) {
    return out << k.name;
}

// Содержимое любого контейнера одной строкой: дерево - с цветами, таблица - отсортированные пары
inline std::string describe(const Container& c) {
    if (const auto* t = dynamic_cast<const RBTree*>(&c)) {
        return join(t->preorder());
    }
    if (const auto* s = dynamic_cast<const Sequence*>(&c)) {
        return join(s->values());
    }
    if (const auto* h = dynamic_cast<const HashTable*>(&c)) {
        return pairs(*h);
    }
    return "";
}

template <typename T>
Sample sample_of(const std::string& name, std::function<void(T&)> fill) {
    return {name, [] { return std::make_unique<T>(); }, [fill] {
                auto c = std::make_unique<T>();
                fill(*c);
                return c;
            }};
}

inline Sample open_sample(const std::string& name, OpenHashTable::Probing probing) {
    return {name, [probing] { return std::make_unique<OpenHashTable>(probing); }, [probing] {
                auto t = std::make_unique<OpenHashTable>(probing);
                t->put("k1", "v1");
                t->put("key with\ttab", "multi\nline");
                return t;
            }};
}

inline std::vector<Sample> samples() {
    return {
        sample_of<Array>("Array", [](Array& a) {
            for (const char* v : {"10", "with space", "tab\there", "line\nbreak", "back\\slash", "quote\"", ""}) {
                a.push_back(v);
            }
        }),
        sample_of<SinglyList>("SinglyList", [](SinglyList& l) {
            l.push_back("a");
            l.push_back("b");
        }),
        sample_of<DoublyList>("DoublyList", [](DoublyList& l) {
            for (const char* v : {"x", "y", "z"}) {
                l.push_back(v);
            }
        }),
        sample_of<Stack>("Stack", [](Stack& s) {
            for (const char* v : {"1", "2", "3"}) {
                s.push(v);
            }
        }),
        sample_of<Queue>("Queue", [](Queue& q) {
            q.push("first");
            q.push("second");
        }),
        sample_of<RBTree>("RBTree", [](RBTree& t) {
            for (const char* v : {"10", "20", "30", "15", "5", "25"}) {
                t.insert(v);
            }
        }),
        sample_of<ChainHashTable>("ChainHashTable", [](ChainHashTable& t) {
            t.put("k1", "v1");
            t.put("key with\ttab", "multi\nline");
        }),
        open_sample("OpenHashTable_linear", OpenHashTable::Probing::Linear),
        open_sample("OpenHashTable_double", OpenHashTable::Probing::Double),
    };
}

inline std::vector<FormatSample> format_samples() {
    std::vector<FormatSample> out;
    for (const auto& s : samples()) {
        out.push_back({s, Format::Binary});
        out.push_back({s, Format::Text});
    }
    return out;
}

inline std::vector<TableKind> table_kinds() {
    return {
        {"Chain", [] { return std::make_unique<ChainHashTable>(); }},
        {"Linear", [] { return std::make_unique<OpenHashTable>(OpenHashTable::Probing::Linear); }},
        {"Double", [] { return std::make_unique<OpenHashTable>(OpenHashTable::Probing::Double); }},
    };
}
