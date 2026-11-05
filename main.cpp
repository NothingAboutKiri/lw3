#include <filesystem>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "array.hpp"
#include "doubly_list.hpp"
#include "hash_table.hpp"
#include "queue.hpp"
#include "rb_tree.hpp"
#include "singly_list.hpp"
#include "stack.hpp"

namespace {

std::string dump(const Sequence& s) {
    const Array items = s.values();
    std::string out = "[";
    for (int i = 0; i < items.length(); i++) {
        out += (i == 0 ? "" : ", ") + items.at(i);
    }
    return out + "]";
}

std::string dump_tree(const RBTree& t) {
    const Array labels = t.preorder();
    std::string out;
    for (int i = 0; i < labels.length(); i++) {
        out += (i == 0 ? "" : " ") + labels.at(i);
    }
    return out;
}

std::string dump(const HashTable& t) {
    std::string out = "{";
    bool first = true;
    t.for_each_pair([&](const std::string& k, const std::string& v) {
        out += (first ? "" : ", ") + k + ": " + v;
        first = false;
    });
    return out + "}";
}

std::string dump(const Container& c) {
    if (const auto* t = dynamic_cast<const RBTree*>(&c)) {
        return dump_tree(*t);
    }
    if (const auto* s = dynamic_cast<const Sequence*>(&c)) {
        return dump(*s);
    }
    return dump(dynamic_cast<const HashTable&>(c));
}

std::unique_ptr<Container> make_empty(const Container& c) {
    const std::string t = c.type_name();
    if (t == "Array") return std::make_unique<Array>();
    if (t == "SinglyList") return std::make_unique<SinglyList>();
    if (t == "DoublyList") return std::make_unique<DoublyList>();
    if (t == "Stack") return std::make_unique<Stack>();
    if (t == "Queue") return std::make_unique<Queue>();
    if (t == "RBTree") return std::make_unique<RBTree>();
    if (t == "ChainHashTable") return std::make_unique<ChainHashTable>();
    if (t == "OpenHashTable/linear") return std::make_unique<OpenHashTable>(OpenHashTable::Probing::Linear);
    return std::make_unique<OpenHashTable>(OpenHashTable::Probing::Double);
}

std::string file_name(const std::string& type) {
    std::string name = type;
    for (char& c : name) {
        if (c == '/') c = '_';
    }
    return name;
}

}

int main(int argc, char* argv[]) {
    const std::filesystem::path dir = argc > 1 ? argv[1] : "data";
    std::filesystem::create_directories(dir);

    auto array = std::make_unique<Array>();
    for (const char* v : {"10", "20", "30"}) array->push_back(v);
    array->insert(1, "15");

    auto slist = std::make_unique<SinglyList>();
    for (const char* v : {"b", "c"}) slist->push_back(v);
    slist->push_front("a");

    auto dlist = std::make_unique<DoublyList>();
    for (const char* v : {"x", "z"}) dlist->push_back(v);
    dlist->insert_before("z", "y");

    auto stack = std::make_unique<Stack>();
    for (const char* v : {"1", "2", "3"}) stack->push(v);

    auto queue = std::make_unique<Queue>();
    for (const char* v : {"first", "second", "third"}) queue->push(v);

    auto tree = std::make_unique<RBTree>();
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) tree->insert(v);

    auto chain = std::make_unique<ChainHashTable>();
    auto linear = std::make_unique<OpenHashTable>(OpenHashTable::Probing::Linear);
    auto dbl = std::make_unique<OpenHashTable>(OpenHashTable::Probing::Double);
    for (HashTable* t : std::initializer_list<HashTable*>{chain.get(), linear.get(), dbl.get()}) {
        t->put("apple", "1");
        t->put("pear", "2");
        t->put("plum", "3");
    }

    std::vector<std::unique_ptr<Container>> all;
    all.push_back(std::move(array));
    all.push_back(std::move(slist));
    all.push_back(std::move(dlist));
    all.push_back(std::move(stack));
    all.push_back(std::move(queue));
    all.push_back(std::move(tree));
    all.push_back(std::move(chain));
    all.push_back(std::move(linear));
    all.push_back(std::move(dbl));

    for (const auto& c : all) {
        std::cout << c->type_name() << " (" << c->length() << "): " << dump(*c) << '\n';
        for (Format const f : {Format::Binary, Format::Text}) {
            const std::string ext = f == Format::Binary ? ".bin" : ".json";
            const auto path = (dir / (file_name(c->type_name()) + ext)).string();
            c->save_file(path, f);
            auto loaded = make_empty(*c);
            loaded->load_file(path, f);
            std::cout << "  " << (f == Format::Binary ? "binary" : "text  ") << " -> " << path << " -> "
                      << dump(*loaded) << (dump(*loaded) == dump(*c) ? "  OK" : "  MISMATCH") << '\n';
        }
    }
    const auto* t = dynamic_cast<const RBTree*>(all[5].get());
    std::cout << "\nRBTree:\n" << t->diagram() << "valid: " << (t->is_valid() ? "yes" : "no") << '\n';
    return 0;
}
