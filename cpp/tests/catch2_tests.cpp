#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "helpers.hpp"

TEST_CASE("PushBackAndGet", "[Array]") {
    Array a;
    CHECK(a.empty());
    for (const char* v : {"10", "20", "30", "40", "50"}) a.push_back(v);
    CHECK(a.length() == 5);
    std::string v;
    CHECK(a.get(0, v));
    CHECK(v == "10");
    CHECK(a.at(4) == "50");
    CHECK_FALSE(a.get(5, v));
    CHECK_FALSE(a.get(-1, v));
    CHECK_FALSE(a.empty());
    CHECK(a.type_name() == "Array");
}

TEST_CASE("InsertAtBeginMiddleEnd", "[Array]") {
    Array a;
    CHECK(a.insert(0, "b"));
    CHECK(a.insert(0, "a"));
    CHECK(a.insert(2, "d"));
    CHECK(a.insert(2, "c"));
    CHECK(join(a) == "a b c d");
    CHECK_FALSE(a.insert(-1, "x"));
    CHECK_FALSE(a.insert(5, "x"));
    CHECK(a.length() == 4);
}

TEST_CASE("SetAndErase", "[Array]") {
    Array a;
    for (const char* v : {"1", "2", "3", "4"}) a.push_back(v);
    CHECK(a.set(0, "one"));
    CHECK_FALSE(a.set(4, "z"));
    CHECK_FALSE(a.set(-1, "z"));
    std::string removed;
    CHECK(a.erase(1, removed));
    CHECK(removed == "2");
    CHECK(join(a) == "one 3 4");
    CHECK(a.erase(2, removed));
    CHECK(removed == "4");
    CHECK_FALSE(a.erase(2, removed));
    CHECK_FALSE(a.erase(-1, removed));
    CHECK(a.length() == 2);
}

TEST_CASE("CopyMoveClearAndGrow", "[Array]") {
    Array a;
    for (int i = 0; i < 1000; i++) a.push_back(std::to_string(i));
    CHECK(a.length() == 1000);
    CHECK(a.at(999) == "999");
    Array copy(a);
    copy.set(0, "changed");
    CHECK(a.at(0) == "0");
    Array assigned;
    assigned = copy;
    CHECK(assigned.at(0) == "changed");
    Array& self = assigned;
    assigned = self;
    CHECK(assigned.length() == 1000);
    Array moved(std::move(copy));
    CHECK(moved.at(0) == "changed");
    copy.push_back("reused");
    CHECK(join(copy) == "reused");
    Array target;
    target = std::move(moved);
    CHECK(target.length() == 1000);
    moved.push_back("again");
    CHECK(moved.length() == 1);
    a.clear();
    CHECK(a.empty());
    a.push_back("again");
    CHECK(join(a) == "again");
    CHECK(join(a.values()) == "again");
}

TEST_CASE("PushFrontBack", "[SinglyList]") {
    SinglyList l;
    std::string v;
    CHECK_FALSE(l.front(v));
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    CHECK(join(l) == "a b c");
    CHECK(l.front(v));
    CHECK(v == "a");
    CHECK(l.length() == 3);
    CHECK(l.type_name() == "SinglyList");
    SinglyList one;
    one.push_front("only");
    CHECK(join(one) == "only");
}

TEST_CASE("InsertAfterBefore", "[SinglyList]") {
    SinglyList l;
    CHECK_FALSE(l.insert_before("x", "y"));
    CHECK_FALSE(l.insert_after("x", "y"));
    l.push_back("a");
    l.push_back("c");
    CHECK(l.insert_after("a", "b"));
    CHECK(l.insert_after("c", "d"));
    CHECK(l.insert_before("a", "start"));
    CHECK(l.insert_before("d", "c2"));
    CHECK_FALSE(l.insert_after("missing", "x"));
    CHECK_FALSE(l.insert_before("missing", "x"));
    CHECK(join(l) == "start a b c c2 d");
    l.push_back("tail");
    CHECK(join(l) == "start a b c c2 d tail");
}

TEST_CASE("PopFrontBack", "[SinglyList]") {
    SinglyList l;
    for (const char* v : {"1", "2", "3"}) l.push_back(v);
    std::string v;
    CHECK(l.pop_front(v));
    CHECK(v == "1");
    CHECK(l.pop_back(v));
    CHECK(v == "3");
    CHECK(l.pop_back(v));
    CHECK(v == "2");
    CHECK(l.empty());
    CHECK_FALSE(l.pop_front(v));
    CHECK_FALSE(l.pop_back(v));
    l.push_back("x");
    CHECK(l.pop_front(v));
    CHECK(l.empty());
    l.push_back("y");
    CHECK(join(l) == "y");
}

TEST_CASE("RemoveContainsGetClear", "[SinglyList]") {
    SinglyList l;
    CHECK_FALSE(l.remove("a"));
    for (const char* v : {"a", "b", "c", "d"}) l.push_back(v);
    CHECK(l.remove("a"));
    CHECK(l.remove("d"));
    CHECK(l.remove("c"));
    CHECK_FALSE(l.remove("zzz"));
    CHECK(l.contains("b"));
    CHECK_FALSE(l.contains("a"));
    std::string v;
    CHECK(l.get(0, v));
    CHECK(v == "b");
    CHECK_FALSE(l.get(1, v));
    CHECK_FALSE(l.get(-1, v));
    l.push_back("e");
    CHECK(join(l) == "b e");
    l.clear();
    CHECK(l.empty());
    l.push_front("z");
    CHECK(join(l) == "z");
}

TEST_CASE("PushAndValues", "[DoublyList]") {
    DoublyList l;
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    CHECK(join(l) == "a b c");
    CHECK(l.type_name() == "DoublyList");
    DoublyList front_only;
    front_only.push_front("2");
    front_only.push_front("1");
    CHECK(join(front_only) == "1 2");
}

TEST_CASE("InsertAfterBefore", "[DoublyList]") {
    DoublyList l;
    CHECK_FALSE(l.insert_after("x", "y"));
    CHECK_FALSE(l.insert_before("x", "y"));
    l.push_back("b");
    CHECK(l.insert_before("b", "a"));
    CHECK(l.insert_after("b", "d"));
    CHECK(l.insert_after("b", "c"));
    CHECK(l.insert_before("d", "c2"));
    CHECK(l.insert_after("d", "e"));
    CHECK(join(l) == "a b c c2 d e");
    std::string v;
    CHECK(l.pop_back(v));
    CHECK(v == "e");
    CHECK(l.pop_front(v));
    CHECK(v == "a");
}

TEST_CASE("PopRemoveGet", "[DoublyList]") {
    DoublyList l;
    std::string v;
    CHECK_FALSE(l.pop_front(v));
    CHECK_FALSE(l.pop_back(v));
    for (const char* s : {"1", "2", "3", "4", "5"}) l.push_back(s);
    CHECK(l.pop_front(v));
    CHECK(v == "1");
    CHECK(l.pop_back(v));
    CHECK(v == "5");
    CHECK(l.remove("3"));
    CHECK_FALSE(l.remove("3"));
    CHECK(l.contains("2"));
    CHECK_FALSE(l.contains("9"));
    CHECK(l.get(1, v));
    CHECK(v == "4");
    CHECK_FALSE(l.get(2, v));
    CHECK_FALSE(l.get(-1, v));
    CHECK(l.remove("2"));
    CHECK(l.remove("4"));
    CHECK(l.empty());
    l.push_back("x");
    CHECK(l.pop_back(v));
    CHECK(l.empty());
    l.push_back("y");
    CHECK(l.pop_front(v));
    CHECK(l.empty());
}

TEST_CASE("Clear", "[DoublyList]") {
    DoublyList l;
    for (const char* v : {"x", "y", "z"}) l.push_back(v);
    l.clear();
    CHECK(l.empty());
    CHECK(join(l) == "");
    l.push_back("again");
    CHECK(l.length() == 1);
}

TEST_CASE("StackIsLifo", "[StackQueue]") {
    Stack s;
    std::string v;
    CHECK_FALSE(s.pop(v));
    CHECK_FALSE(s.top(v));
    for (const char* x : {"1", "2", "3"}) s.push(x);
    CHECK(s.top(v));
    CHECK(v == "3");
    CHECK(join(s) == "3 2 1");
    CHECK(s.pop(v));
    CHECK(v == "3");
    CHECK(s.pop(v));
    CHECK(v == "2");
    CHECK(s.length() == 1);
    CHECK(s.type_name() == "Stack");
    s.clear();
    CHECK(s.empty());
}

TEST_CASE("QueueIsFifo", "[StackQueue]") {
    Queue q;
    std::string v;
    CHECK_FALSE(q.pop(v));
    CHECK_FALSE(q.front(v));
    for (const char* x : {"a", "b", "c"}) q.push(x);
    CHECK(q.front(v));
    CHECK(v == "a");
    CHECK(q.pop(v));
    CHECK(v == "a");
    q.push("d");
    CHECK(join(q) == "b c d");
    CHECK(q.length() == 3);
    CHECK(q.type_name() == "Queue");
    q.clear();
    CHECK(q.empty());
}

TEST_CASE("InsertBalancesLikeLab1", "[RBTree]") {
    RBTree t;
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) {
        CHECK(t.insert(v));
    }
    CHECK_FALSE(t.insert("15"));
    CHECK(join(t.preorder()) == "20(B) 10(B) 5(R) 15(R) 30(B) 25(R)");
    CHECK(join(t.inorder()) == "5(R) 10(B) 15(R) 20(B) 25(R) 30(B)");
    CHECK(join(t.postorder()) == "5(R) 15(R) 10(B) 25(R) 30(B) 20(B)");
    CHECK(join(t.bfs()) == "20(B) 10(B) 30(B) 5(R) 15(R) 25(R)");
    CHECK(join(t.keys()) == "5 10 15 20 25 30");
    CHECK(join(t) == "5 10 15 20 25 30");
    CHECK(t.is_valid());
    CHECK(t.length() == 6);
    CHECK(t.type_name() == "RBTree");
}

TEST_CASE("SearchMinMaxNextPrev", "[RBTree]") {
    RBTree t;
    std::string v;
    CHECK_FALSE(t.min_key(v));
    CHECK_FALSE(t.max_key(v));
    for (const char* s : {"10", "20", "30", "15", "5", "25"}) t.insert(s);
    CHECK(t.contains("25"));
    CHECK_FALSE(t.contains("99"));
    CHECK(t.min_key(v));
    CHECK(v == "5");
    CHECK(t.max_key(v));
    CHECK(v == "30");
    CHECK(t.next("15", v));
    CHECK(v == "20");
    CHECK(t.next("20", v));
    CHECK(v == "25");
    CHECK(t.prev("15", v));
    CHECK(v == "10");
    CHECK(t.prev("20", v));
    CHECK(v == "15");
    CHECK(t.prev("25", v));
    CHECK(v == "20");
    CHECK_FALSE(t.next("30", v));
    CHECK_FALSE(t.prev("5", v));
    CHECK_FALSE(t.next("99", v));
    CHECK_FALSE(t.prev("99", v));
}

TEST_CASE("EraseAllCases", "[RBTree]") {
    RBTree t;
    CHECK_FALSE(t.erase("1"));
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) t.insert(v);
    CHECK(t.erase("20"));
    CHECK(join(t.preorder()) == "25(B) 10(B) 5(R) 15(R) 30(B)");
    CHECK(t.erase("10"));
    CHECK(join(t.preorder()) == "25(B) 15(B) 5(R) 30(B)");
    CHECK_FALSE(t.erase("99"));
    CHECK(t.erase("5"));
    CHECK(t.erase("30"));
    CHECK(t.erase("25"));
    CHECK(t.erase("15"));
    CHECK(t.empty());
    CHECK(t.is_valid());
}

TEST_CASE("StaysValidUnderRandomOps", "[RBTree]") {
    RBTree t;
    unsigned x = 12345;
    int inserted = 0;
    for (int i = 0; i < 3000; i++) {
        x = x * 1103515245U + 12345U;
        std::string key = std::to_string((x >> 8) % 500);
        if ((x >> 4) % 3 == 0) t.erase(key); else inserted += t.insert(key) ? 1 : 0;
    }
    CHECK(inserted > 0);
    CHECK(t.is_valid());
}

TEST_CASE("OrderingAndDiagram", "[RBTree]") {
    RBTree t;
    CHECK(t.diagram() == "(empty tree)\n");
    CHECK(join(t.bfs()) == "");
    for (const char* v : {"2", "10", "-5", "abc", "99"}) t.insert(v);
    CHECK(join(t.keys()) == "-5 2 10 99 abc");
    RBTree d;
    for (const char* v : {"2", "1", "3", "4"}) d.insert(v);
    CHECK(d.diagram() == "2(B)\n|-- L: 1(B)\n`-- R: 3(B)\n    `-- R: 4(R)\n");
    RBTree left;
    for (const char* v : {"3", "2", "1", "0"}) left.insert(v);
    CHECK(left.diagram() == "2(B)\n|-- L: 1(B)\n|   `-- L: 0(R)\n`-- R: 3(B)\n");
    left.clear();
    CHECK(left.empty());
    CHECK(left.insert("z"));
    CHECK(left.contains("z"));
}

TEST_CASE("HashFunction", "[HashTable]") {
    CHECK(hash_string("") == 0U);
    CHECK(hash_string("a") == 97U);
    CHECK(hash_string("ab") == (97U * 31U + 98U));
    CHECK(hash_string("ab") != hash_string("ba"));
}

TEST_CASE("PutGetRemove", "[HashTable]") {
    const TableKind param = GENERATE(Catch::Generators::from_range(table_kinds()));
    auto t = param.make();
    CHECK(t->empty());
    CHECK_FALSE(t->get("x").has_value());
    CHECK_FALSE(t->remove("x"));
    CHECK(t->put("apple", "1"));
    CHECK(t->put("pear", "2"));
    CHECK_FALSE(t->put("apple", "10"));
    CHECK(t->get("apple").value_or("-") == "10");
    CHECK(t->contains("pear"));
    CHECK_FALSE(t->contains("plum"));
    CHECK(t->length() == 2);
    CHECK(t->load_factor() > 0.0);
    CHECK(t->remove("pear"));
    CHECK_FALSE(t->remove("pear"));
    CHECK_FALSE(t->contains("pear"));
    CHECK((pairs(*t)) == "apple=10");
    t->clear();
    CHECK(t->empty());
    CHECK(t->put("after", "clear"));
}

TEST_CASE("GrowsAndKeepsAllKeys", "[HashTable]") {
    const TableKind param = GENERATE(Catch::Generators::from_range(table_kinds()));
    auto t = param.make();
    const int start = t->capacity();
    for (int i = 0; i < 2000; i++) t->put("key" + std::to_string(i), std::to_string(i));
    CHECK(t->length() == 2000);
    CHECK(t->capacity() > start);
    CHECK(t->load_factor() <= 0.75);
    bool all = true;
    for (int i = 0; i < 2000; i++) all = all && t->get("key" + std::to_string(i)).value_or("") == std::to_string(i);
    CHECK(all);
    for (int i = 0; i < 2000; i += 2) t->remove("key" + std::to_string(i));
    CHECK(t->length() == 1000);
    CHECK_FALSE(t->contains("key0"));
    CHECK(t->contains("key1"));
}

TEST_CASE("ChainSpecifics", "[HashTable]") {
    ChainHashTable t(4);
    CHECK(t.capacity() == 4);
    CHECK(t.type_name() == "ChainHashTable");
    for (const char* k : {"a", "b", "c", "d", "e"}) t.put(k, k);
    CHECK(t.capacity() >= 8);
    CHECK(t.longest_chain() >= 1);
    ChainHashTable zero(0);
    CHECK(zero.capacity() == 8);
    ChainHashTable copy(t);
    copy.put("f", "f");
    CHECK(t.length() == 5);
    ChainHashTable assigned;
    assigned = copy;
    ChainHashTable& self = assigned;
    assigned = self;
    CHECK(assigned.length() == 6);
    ChainHashTable moved(std::move(assigned));
    CHECK(moved.get("f").value_or("-") == "f");
    CHECK_FALSE(assigned.get("f").has_value());
    CHECK_FALSE(assigned.remove("f"));
    CHECK(assigned.put("x", "1"));
    ChainHashTable target;
    target = std::move(moved);
    CHECK(target.length() == 6);
}

TEST_CASE("OpenAddressingTombstones", "[HashTable]") {
    OpenHashTable lin(OpenHashTable::Probing::Linear, 11);
    OpenHashTable dbl(OpenHashTable::Probing::Double, 11);
    CHECK(lin.type_name() == "OpenHashTable/linear");
    CHECK(dbl.type_name() == "OpenHashTable/double");
    CHECK(lin.probing() == OpenHashTable::Probing::Linear);
    for (OpenHashTable* t : {&lin, &dbl}) {
        for (const char* k : {"pear", "lime", "grape"}) t->put(k, k);
        CHECK(t->capacity() == 11);
        CHECK(t->probes("grape") >= 2);
        CHECK(t->remove("lime"));
        CHECK(t->get("grape").value_or("-") == "grape");
        CHECK(t->put("lime", "again"));
        CHECK(t->get("lime").value_or("-") == "again");
        CHECK(t->probes("missing") >= 1);
    }
    CHECK(lin.probes("grape") == 3);
    CHECK(dbl.probes("grape") == 2);
}

TEST_CASE("OpenHelpersAndCopies", "[HashTable]") {
    CHECK(OpenHashTable::is_prime(2));
    CHECK(OpenHashTable::is_prime(1009));
    CHECK_FALSE(OpenHashTable::is_prime(1));
    CHECK_FALSE(OpenHashTable::is_prime(1000));
    CHECK(OpenHashTable::next_prime(1000) == 1009);
    OpenHashTable small(OpenHashTable::Probing::Linear, 1);
    CHECK(small.capacity() == 3);
    OpenHashTable t(OpenHashTable::Probing::Double, 11);
    for (int i = 0; i < 30; i++) {
        t.put("k" + std::to_string(i), "v");
        t.remove("k" + std::to_string(i));
    }
    CHECK(t.length() == 0);
    CHECK(t.put("alive", "1"));
    OpenHashTable copy(t);
    copy.put("other", "2");
    CHECK(t.length() == 1);
    OpenHashTable assigned;
    assigned = copy;
    OpenHashTable& self = assigned;
    assigned = self;
    CHECK(assigned.length() == 2);
    OpenHashTable moved(std::move(assigned));
    CHECK(moved.get("other").value_or("-") == "2");
    CHECK_FALSE(assigned.get("other").has_value());
    CHECK_FALSE(assigned.remove("other"));
    CHECK(assigned.probes("other") == 0);
    CHECK(assigned.put("x", "1"));
    OpenHashTable target;
    target = std::move(moved);
    CHECK(target.length() == 2);
}

TEST_CASE("Roundtrip", "[Serialization]") {
    const FormatSample param = GENERATE(Catch::Generators::from_range(format_samples()));
    auto original = param.sample.make_filled();
    std::stringstream buffer;
    original->save(buffer, param.format);
    auto loaded = param.sample.make_empty();
    loaded->load(buffer, param.format);
    CHECK(loaded->type_name() == original->type_name());
    CHECK(loaded->length() == original->length());
    CHECK((describe(*loaded)) == (describe(*original)));
}

TEST_CASE("EmptyContainer", "[Serialization]") {
    const Sample param = GENERATE(Catch::Generators::from_range(samples()));
    auto c = param.make_empty();
    CHECK(c->empty());
    std::stringstream out;
    c->save(out, Format::Text);
    CHECK(out.str() == ("{\n  \"" + c->type_name() + "\": []\n}\n"));
    c->load(out, Format::Text);
    CHECK(c->length() == 0);
}

TEST_CASE("RejectsBadJson", "[Serialization]") {
    const std::string param = GENERATE(Catch::Generators::from_range(std::vector<std::string>{
    "", "[]", "{}", "{\"Array\": [\"\\ud800\"]}", "{\"Array\": [\"1\"", "{\"Array\": [\"1\" \"2\"]}",
    "{\"Array\" [\"1\"]}", "{\"Array\": [\"bad\\q\"]}", "{\"Array\": [\"\\u12\"]}",
    "{\"Array\": [\"open", "{\"Queue\": [\"x\"]}", "{\"Array\": [], \"Queue\": []}",
    "{\"Array\": []} tail"}));
    Array a;
    a.push_back("old");
    std::stringstream in(param);
    CHECK_THROWS_AS(a.load(in, Format::Text), std::runtime_error);
    CHECK(join(a) == "old");
}

TEST_CASE("RejectsBadTree", "[Serialization]") {
    const std::string param = GENERATE(Catch::Generators::from_range(std::vector<std::string>{
    "{\"RBTree\": [\"5(X)\"]}", "{\"RBTree\": [\"5\"]}", "{\"RBTree\": [\"5(R)\"]}",
    "{\"RBTree\": [\"5(B)\", \"5(R)\"]}", "{\"RBTree\": [\"5(B)\", \"3(B)\"]}"}));
    RBTree t;
    std::stringstream good("{\"RBTree\": [\"5(B)\", \"3(R)\", \"8(R)\"]}");
    CHECK_NOTHROW(t.load(good, Format::Text));
    std::stringstream in(param);
    CHECK_THROWS_AS(t.load(in, Format::Text), std::runtime_error);
    CHECK(join(t.preorder()) == "5(B) 3(R) 8(R)");
}

TEST_CASE("TextFormatIsLab1Json", "[Serialization]") {
    SinglyList sl;
    for (const char* v : {"a", "b"}) sl.push_back(v);
    CHECK(text_of(sl) == "{\n  \"SinglyList\": [\"a\", \"b\"]\n}\n");
    RBTree t;
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) t.insert(v);
    CHECK(text_of(t) == "{\n  \"RBTree\": [\"20(B)\", \"10(B)\", \"5(R)\", \"15(R)\", \"30(B)\", \"25(R)\"]\n}\n");
    ChainHashTable h;
    h.put("k", "v");
    CHECK(text_of(h) == "{\n  \"ChainHashTable\": [\"k\", \"v\"]\n}\n");
    Stack st;
    for (const char* v : {"1", "2", "3"}) st.push(v);
    Stack st2;
    roundtrip(st, st2, Format::Text);
    std::string v;
    CHECK(st2.pop(v));
    CHECK(v == "3");
}

TEST_CASE("JsonParsingLikeLab1", "[Serialization]") {
    Array a;
    std::stringstream spaced(" {  \"Array\" :\n [ \"1\" ,\t\"2\" ] } ");
    CHECK_NOTHROW(a.load(spaced, Format::Text));
    CHECK(join(a) == "1 2");
    std::stringstream numbers("{\"Array\": [15, -3]}");
    CHECK_NOTHROW(a.load(numbers, Format::Text));
    CHECK(join(a) == "15 -3");
    std::stringstream escapes("{\"Array\": [\"a\\\"b\", \"c\\\\d\", \"e\\nf\", \"\\t\\r\\/\\b\\f\", \"\\u0041\\u00e9\\u20ac\"]}");
    CHECK_NOTHROW(a.load(escapes, Format::Text));
    CHECK(a.length() == 5);
    CHECK(a.at(0) == "a\"b");
    CHECK(a.at(1) == "c\\d");
    CHECK(a.at(2) == "e\nf");
    CHECK(a.at(4) == "A\xc3\xa9\xe2\x82\xac");
    Array ctrl;
    ctrl.push_back(std::string(1, '\x01'));
    Array ctrl2;
    roundtrip(ctrl, ctrl2, Format::Text);
    CHECK(ctrl2.at(0) == std::string(1, '\x01'));
}

TEST_CASE("FilesRoundtrip", "[Serialization]") {
    const std::string bin = "lab3_test_array.bin";
    const std::string txt = "lab3_test_array.json";
    Array a;
    for (const char* v : {"1", "2", "3"}) a.push_back(v);
    a.save_file(bin, Format::Binary);
    a.save_file(txt, Format::Text);
    Array b;
    b.load_file(bin, Format::Binary);
    CHECK(join(b) == "1 2 3");
    Array c;
    c.load_file(txt, Format::Text);
    CHECK(join(c) == "1 2 3");
    std::remove(bin.c_str());
    std::remove(txt.c_str());
    CHECK_THROWS_AS(c.load_file("no/such/dir/file.bin", Format::Binary), std::runtime_error);
    CHECK_THROWS_AS(c.save_file("no/such/dir/file.bin", Format::Binary), std::runtime_error);
}

TEST_CASE("RejectsBadBinary", "[Serialization]") {
    Array a;
    a.push_back("x");
    std::stringstream bin;
    a.save(bin, Format::Binary);
    SinglyList wrong_type;
    CHECK_THROWS_AS(wrong_type.load(bin, Format::Binary), std::runtime_error);
    std::stringstream garbage("XXXXrest");
    CHECK_THROWS_AS(a.load(garbage, Format::Binary), std::runtime_error);
    std::string full;
    {
        std::stringstream b2;
        a.save(b2, Format::Binary);
        full = b2.str();
    }
    std::stringstream truncated(full.substr(0, full.size() - 1));
    CHECK_THROWS_AS(a.load(truncated, Format::Binary), std::runtime_error);
    CHECK(join(a) == "x");
    std::stringstream short_header(full.substr(0, 6));
    CHECK_THROWS_AS(a.load(short_header, Format::Binary), std::runtime_error);
    std::stringstream huge;
    huge.write("L3B1", 4);
    const char len_type[4] = {5, 0, 0, 0};
    huge.write(len_type, 4);
    huge.write("Array", 5);
    const char one[4] = {1, 0, 0, 0};
    huge.write(one, 4);
    const char big[4] = {0, 0, 0, 0x7f};
    huge.write(big, 4);
    CHECK_THROWS_AS(a.load(huge, Format::Binary), std::runtime_error);
    RBTree t;
    t.insert("1");
    t.insert("2");
    std::stringstream tb;
    t.save(tb, Format::Binary);
    std::string data = tb.str();
    std::stringstream cut(data.substr(0, data.size() - 2));
    RBTree t2;
    CHECK_THROWS_AS(t2.load(cut, Format::Binary), std::runtime_error);
    CHECK(t2.empty());
    ChainHashTable h;
    std::stringstream odd("{\"ChainHashTable\": [\"key\"]}");
    CHECK_THROWS_AS(h.load(odd, Format::Text), std::runtime_error);
}
