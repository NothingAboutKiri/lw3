#define BOOST_TEST_MODULE lab3_boost
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "helpers.hpp"

BOOST_AUTO_TEST_SUITE(ArrayTests)

BOOST_AUTO_TEST_CASE(PushBackAndGet) {
    Array a;
    BOOST_CHECK(a.empty());
    for (const char* v : {"10", "20", "30", "40", "50"}) a.push_back(v);
    BOOST_CHECK_EQUAL(a.length(), 5);
    std::string v;
    BOOST_CHECK(a.get(0, v));
    BOOST_CHECK_EQUAL(v, "10");
    BOOST_CHECK_EQUAL(a.at(4), "50");
    BOOST_CHECK(!(a.get(5, v)));
    BOOST_CHECK(!(a.get(-1, v)));
    BOOST_CHECK(!(a.empty()));
    BOOST_CHECK_EQUAL(a.type_name(), "Array");
}

BOOST_AUTO_TEST_CASE(InsertAtBeginMiddleEnd) {
    Array a;
    BOOST_CHECK(a.insert(0, "b"));
    BOOST_CHECK(a.insert(0, "a"));
    BOOST_CHECK(a.insert(2, "d"));
    BOOST_CHECK(a.insert(2, "c"));
    BOOST_CHECK_EQUAL(join(a), "a b c d");
    BOOST_CHECK(!(a.insert(-1, "x")));
    BOOST_CHECK(!(a.insert(5, "x")));
    BOOST_CHECK_EQUAL(a.length(), 4);
}

BOOST_AUTO_TEST_CASE(SetAndErase) {
    Array a;
    for (const char* v : {"1", "2", "3", "4"}) a.push_back(v);
    BOOST_CHECK(a.set(0, "one"));
    BOOST_CHECK(!(a.set(4, "z")));
    BOOST_CHECK(!(a.set(-1, "z")));
    std::string removed;
    BOOST_CHECK(a.erase(1, removed));
    BOOST_CHECK_EQUAL(removed, "2");
    BOOST_CHECK_EQUAL(join(a), "one 3 4");
    BOOST_CHECK(a.erase(2, removed));
    BOOST_CHECK_EQUAL(removed, "4");
    BOOST_CHECK(!(a.erase(2, removed)));
    BOOST_CHECK(!(a.erase(-1, removed)));
    BOOST_CHECK_EQUAL(a.length(), 2);
}

BOOST_AUTO_TEST_CASE(CopyMoveClearAndGrow) {
    Array a;
    for (int i = 0; i < 1000; i++) a.push_back(std::to_string(i));
    BOOST_CHECK_EQUAL(a.length(), 1000);
    BOOST_CHECK_EQUAL(a.at(999), "999");
    Array copy(a);
    copy.set(0, "changed");
    BOOST_CHECK_EQUAL(a.at(0), "0");
    Array assigned;
    assigned = copy;
    BOOST_CHECK_EQUAL(assigned.at(0), "changed");
    Array& self = assigned;
    assigned = self;
    BOOST_CHECK_EQUAL(assigned.length(), 1000);
    Array moved(std::move(copy));
    BOOST_CHECK_EQUAL(moved.at(0), "changed");
    copy.push_back("reused");
    BOOST_CHECK_EQUAL(join(copy), "reused");
    Array target;
    target = std::move(moved);
    BOOST_CHECK_EQUAL(target.length(), 1000);
    moved.push_back("again");
    BOOST_CHECK_EQUAL(moved.length(), 1);
    a.clear();
    BOOST_CHECK(a.empty());
    a.push_back("again");
    BOOST_CHECK_EQUAL(join(a), "again");
    BOOST_CHECK_EQUAL(join(a.values()), "again");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SinglyListTests)

BOOST_AUTO_TEST_CASE(PushFrontBack) {
    SinglyList l;
    std::string v;
    BOOST_CHECK(!(l.front(v)));
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    BOOST_CHECK_EQUAL(join(l), "a b c");
    BOOST_CHECK(l.front(v));
    BOOST_CHECK_EQUAL(v, "a");
    BOOST_CHECK_EQUAL(l.length(), 3);
    BOOST_CHECK_EQUAL(l.type_name(), "SinglyList");
    SinglyList one;
    one.push_front("only");
    BOOST_CHECK_EQUAL(join(one), "only");
}

BOOST_AUTO_TEST_CASE(InsertAfterBefore) {
    SinglyList l;
    BOOST_CHECK(!(l.insert_before("x", "y")));
    BOOST_CHECK(!(l.insert_after("x", "y")));
    l.push_back("a");
    l.push_back("c");
    BOOST_CHECK(l.insert_after("a", "b"));
    BOOST_CHECK(l.insert_after("c", "d"));
    BOOST_CHECK(l.insert_before("a", "start"));
    BOOST_CHECK(l.insert_before("d", "c2"));
    BOOST_CHECK(!(l.insert_after("missing", "x")));
    BOOST_CHECK(!(l.insert_before("missing", "x")));
    BOOST_CHECK_EQUAL(join(l), "start a b c c2 d");
    l.push_back("tail");
    BOOST_CHECK_EQUAL(join(l), "start a b c c2 d tail");
}

BOOST_AUTO_TEST_CASE(PopFrontBack) {
    SinglyList l;
    for (const char* v : {"1", "2", "3"}) l.push_back(v);
    std::string v;
    BOOST_CHECK(l.pop_front(v));
    BOOST_CHECK_EQUAL(v, "1");
    BOOST_CHECK(l.pop_back(v));
    BOOST_CHECK_EQUAL(v, "3");
    BOOST_CHECK(l.pop_back(v));
    BOOST_CHECK_EQUAL(v, "2");
    BOOST_CHECK(l.empty());
    BOOST_CHECK(!(l.pop_front(v)));
    BOOST_CHECK(!(l.pop_back(v)));
    l.push_back("x");
    BOOST_CHECK(l.pop_front(v));
    BOOST_CHECK(l.empty());
    l.push_back("y");
    BOOST_CHECK_EQUAL(join(l), "y");
}

BOOST_AUTO_TEST_CASE(RemoveContainsGetClear) {
    SinglyList l;
    BOOST_CHECK(!(l.remove("a")));
    for (const char* v : {"a", "b", "c", "d"}) l.push_back(v);
    BOOST_CHECK(l.remove("a"));
    BOOST_CHECK(l.remove("d"));
    BOOST_CHECK(l.remove("c"));
    BOOST_CHECK(!(l.remove("zzz")));
    BOOST_CHECK(l.contains("b"));
    BOOST_CHECK(!(l.contains("a")));
    std::string v;
    BOOST_CHECK(l.get(0, v));
    BOOST_CHECK_EQUAL(v, "b");
    BOOST_CHECK(!(l.get(1, v)));
    BOOST_CHECK(!(l.get(-1, v)));
    l.push_back("e");
    BOOST_CHECK_EQUAL(join(l), "b e");
    l.clear();
    BOOST_CHECK(l.empty());
    l.push_front("z");
    BOOST_CHECK_EQUAL(join(l), "z");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(DoublyListTests)

BOOST_AUTO_TEST_CASE(PushAndValues) {
    DoublyList l;
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    BOOST_CHECK_EQUAL(join(l), "a b c");
    BOOST_CHECK_EQUAL(l.type_name(), "DoublyList");
    DoublyList front_only;
    front_only.push_front("2");
    front_only.push_front("1");
    BOOST_CHECK_EQUAL(join(front_only), "1 2");
}

BOOST_AUTO_TEST_CASE(InsertAfterBefore) {
    DoublyList l;
    BOOST_CHECK(!(l.insert_after("x", "y")));
    BOOST_CHECK(!(l.insert_before("x", "y")));
    l.push_back("b");
    BOOST_CHECK(l.insert_before("b", "a"));
    BOOST_CHECK(l.insert_after("b", "d"));
    BOOST_CHECK(l.insert_after("b", "c"));
    BOOST_CHECK(l.insert_before("d", "c2"));
    BOOST_CHECK(l.insert_after("d", "e"));
    BOOST_CHECK_EQUAL(join(l), "a b c c2 d e");
    std::string v;
    BOOST_CHECK(l.pop_back(v));
    BOOST_CHECK_EQUAL(v, "e");
    BOOST_CHECK(l.pop_front(v));
    BOOST_CHECK_EQUAL(v, "a");
}

BOOST_AUTO_TEST_CASE(PopRemoveGet) {
    DoublyList l;
    std::string v;
    BOOST_CHECK(!(l.pop_front(v)));
    BOOST_CHECK(!(l.pop_back(v)));
    for (const char* s : {"1", "2", "3", "4", "5"}) l.push_back(s);
    BOOST_CHECK(l.pop_front(v));
    BOOST_CHECK_EQUAL(v, "1");
    BOOST_CHECK(l.pop_back(v));
    BOOST_CHECK_EQUAL(v, "5");
    BOOST_CHECK(l.remove("3"));
    BOOST_CHECK(!(l.remove("3")));
    BOOST_CHECK(l.contains("2"));
    BOOST_CHECK(!(l.contains("9")));
    BOOST_CHECK(l.get(1, v));
    BOOST_CHECK_EQUAL(v, "4");
    BOOST_CHECK(!(l.get(2, v)));
    BOOST_CHECK(!(l.get(-1, v)));
    BOOST_CHECK(l.remove("2"));
    BOOST_CHECK(l.remove("4"));
    BOOST_CHECK(l.empty());
    l.push_back("x");
    BOOST_CHECK(l.pop_back(v));
    BOOST_CHECK(l.empty());
    l.push_back("y");
    BOOST_CHECK(l.pop_front(v));
    BOOST_CHECK(l.empty());
}

BOOST_AUTO_TEST_CASE(Clear) {
    DoublyList l;
    for (const char* v : {"x", "y", "z"}) l.push_back(v);
    l.clear();
    BOOST_CHECK(l.empty());
    BOOST_CHECK_EQUAL(join(l), "");
    l.push_back("again");
    BOOST_CHECK_EQUAL(l.length(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(StackQueueTests)

BOOST_AUTO_TEST_CASE(StackIsLifo) {
    Stack s;
    std::string v;
    BOOST_CHECK(!(s.pop(v)));
    BOOST_CHECK(!(s.top(v)));
    for (const char* x : {"1", "2", "3"}) s.push(x);
    BOOST_CHECK(s.top(v));
    BOOST_CHECK_EQUAL(v, "3");
    BOOST_CHECK_EQUAL(join(s), "3 2 1");
    BOOST_CHECK(s.pop(v));
    BOOST_CHECK_EQUAL(v, "3");
    BOOST_CHECK(s.pop(v));
    BOOST_CHECK_EQUAL(v, "2");
    BOOST_CHECK_EQUAL(s.length(), 1);
    BOOST_CHECK_EQUAL(s.type_name(), "Stack");
    s.clear();
    BOOST_CHECK(s.empty());
}

BOOST_AUTO_TEST_CASE(QueueIsFifo) {
    Queue q;
    std::string v;
    BOOST_CHECK(!(q.pop(v)));
    BOOST_CHECK(!(q.front(v)));
    for (const char* x : {"a", "b", "c"}) q.push(x);
    BOOST_CHECK(q.front(v));
    BOOST_CHECK_EQUAL(v, "a");
    BOOST_CHECK(q.pop(v));
    BOOST_CHECK_EQUAL(v, "a");
    q.push("d");
    BOOST_CHECK_EQUAL(join(q), "b c d");
    BOOST_CHECK_EQUAL(q.length(), 3);
    BOOST_CHECK_EQUAL(q.type_name(), "Queue");
    q.clear();
    BOOST_CHECK(q.empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(RBTreeTests)

BOOST_AUTO_TEST_CASE(InsertBalancesLikeLab1) {
    RBTree t;
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) {
        BOOST_CHECK(t.insert(v));
    }
    BOOST_CHECK(!(t.insert("15")));
    BOOST_CHECK_EQUAL(join(t.preorder()), "20(B) 10(B) 5(R) 15(R) 30(B) 25(R)");
    BOOST_CHECK_EQUAL(join(t.inorder()), "5(R) 10(B) 15(R) 20(B) 25(R) 30(B)");
    BOOST_CHECK_EQUAL(join(t.postorder()), "5(R) 15(R) 10(B) 25(R) 30(B) 20(B)");
    BOOST_CHECK_EQUAL(join(t.bfs()), "20(B) 10(B) 30(B) 5(R) 15(R) 25(R)");
    BOOST_CHECK_EQUAL(join(t.keys()), "5 10 15 20 25 30");
    BOOST_CHECK_EQUAL(join(t), "5 10 15 20 25 30");
    BOOST_CHECK(t.is_valid());
    BOOST_CHECK_EQUAL(t.length(), 6);
    BOOST_CHECK_EQUAL(t.type_name(), "RBTree");
}

BOOST_AUTO_TEST_CASE(SearchMinMaxNextPrev) {
    RBTree t;
    std::string v;
    BOOST_CHECK(!(t.min_key(v)));
    BOOST_CHECK(!(t.max_key(v)));
    for (const char* s : {"10", "20", "30", "15", "5", "25"}) t.insert(s);
    BOOST_CHECK(t.contains("25"));
    BOOST_CHECK(!(t.contains("99")));
    BOOST_CHECK(t.min_key(v));
    BOOST_CHECK_EQUAL(v, "5");
    BOOST_CHECK(t.max_key(v));
    BOOST_CHECK_EQUAL(v, "30");
    BOOST_CHECK(t.next("15", v));
    BOOST_CHECK_EQUAL(v, "20");
    BOOST_CHECK(t.next("20", v));
    BOOST_CHECK_EQUAL(v, "25");
    BOOST_CHECK(t.prev("15", v));
    BOOST_CHECK_EQUAL(v, "10");
    BOOST_CHECK(t.prev("20", v));
    BOOST_CHECK_EQUAL(v, "15");
    BOOST_CHECK(t.prev("25", v));
    BOOST_CHECK_EQUAL(v, "20");
    BOOST_CHECK(!(t.next("30", v)));
    BOOST_CHECK(!(t.prev("5", v)));
    BOOST_CHECK(!(t.next("99", v)));
    BOOST_CHECK(!(t.prev("99", v)));
}

BOOST_AUTO_TEST_CASE(EraseAllCases) {
    RBTree t;
    BOOST_CHECK(!(t.erase("1")));
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) t.insert(v);
    BOOST_CHECK(t.erase("20"));
    BOOST_CHECK_EQUAL(join(t.preorder()), "25(B) 10(B) 5(R) 15(R) 30(B)");
    BOOST_CHECK(t.erase("10"));
    BOOST_CHECK_EQUAL(join(t.preorder()), "25(B) 15(B) 5(R) 30(B)");
    BOOST_CHECK(!(t.erase("99")));
    BOOST_CHECK(t.erase("5"));
    BOOST_CHECK(t.erase("30"));
    BOOST_CHECK(t.erase("25"));
    BOOST_CHECK(t.erase("15"));
    BOOST_CHECK(t.empty());
    BOOST_CHECK(t.is_valid());
}

BOOST_AUTO_TEST_CASE(StaysValidUnderRandomOps) {
    RBTree t;
    unsigned x = 12345;
    int inserted = 0;
    for (int i = 0; i < 3000; i++) {
        x = x * 1103515245U + 12345U;
        std::string key = std::to_string((x >> 8) % 500);
        if ((x >> 4) % 3 == 0) t.erase(key); else inserted += t.insert(key) ? 1 : 0;
    }
    BOOST_CHECK(inserted > 0);
    BOOST_CHECK(t.is_valid());
}

BOOST_AUTO_TEST_CASE(OrderingAndDiagram) {
    RBTree t;
    BOOST_CHECK_EQUAL(t.diagram(), "(empty tree)\n");
    BOOST_CHECK_EQUAL(join(t.bfs()), "");
    for (const char* v : {"2", "10", "-5", "abc", "99"}) t.insert(v);
    BOOST_CHECK_EQUAL(join(t.keys()), "-5 2 10 99 abc");
    RBTree d;
    for (const char* v : {"2", "1", "3", "4"}) d.insert(v);
    BOOST_CHECK_EQUAL(d.diagram(), "2(B)\n|-- L: 1(B)\n`-- R: 3(B)\n    `-- R: 4(R)\n");
    RBTree left;
    for (const char* v : {"3", "2", "1", "0"}) left.insert(v);
    BOOST_CHECK_EQUAL(left.diagram(), "2(B)\n|-- L: 1(B)\n|   `-- L: 0(R)\n`-- R: 3(B)\n");
    left.clear();
    BOOST_CHECK(left.empty());
    BOOST_CHECK(left.insert("z"));
    BOOST_CHECK(left.contains("z"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(HashTableTests)

BOOST_AUTO_TEST_CASE(HashFunction) {
    BOOST_CHECK_EQUAL(hash_string(""), 0U);
    BOOST_CHECK_EQUAL(hash_string("a"), 97U);
    BOOST_CHECK_EQUAL(hash_string("ab"), 97U * 31U + 98U);
    BOOST_CHECK(hash_string("ab") != hash_string("ba"));
}

BOOST_DATA_TEST_CASE(PutGetRemove, boost::unit_test::data::make(table_kinds()), param) {
    auto t = param.make();
    BOOST_CHECK(t->empty());
    BOOST_CHECK(!(t->get("x").has_value()));
    BOOST_CHECK(!(t->remove("x")));
    BOOST_CHECK(t->put("apple", "1"));
    BOOST_CHECK(t->put("pear", "2"));
    BOOST_CHECK(!(t->put("apple", "10")));
    BOOST_CHECK_EQUAL(t->get("apple").value_or("-"), "10");
    BOOST_CHECK(t->contains("pear"));
    BOOST_CHECK(!(t->contains("plum")));
    BOOST_CHECK_EQUAL(t->length(), 2);
    BOOST_CHECK(t->load_factor() > 0.0);
    BOOST_CHECK(t->remove("pear"));
    BOOST_CHECK(!(t->remove("pear")));
    BOOST_CHECK(!(t->contains("pear")));
    BOOST_CHECK_EQUAL(pairs(*t), "apple=10");
    t->clear();
    BOOST_CHECK(t->empty());
    BOOST_CHECK(t->put("after", "clear"));
}

BOOST_DATA_TEST_CASE(GrowsAndKeepsAllKeys, boost::unit_test::data::make(table_kinds()), param) {
    auto t = param.make();
    const int start = t->capacity();
    for (int i = 0; i < 2000; i++) t->put("key" + std::to_string(i), std::to_string(i));
    BOOST_CHECK_EQUAL(t->length(), 2000);
    BOOST_CHECK(t->capacity() > start);
    BOOST_CHECK(t->load_factor() <= 0.75);
    bool all = true;
    for (int i = 0; i < 2000; i++) all = all && t->get("key" + std::to_string(i)).value_or("") == std::to_string(i);
    BOOST_CHECK(all);
    for (int i = 0; i < 2000; i += 2) t->remove("key" + std::to_string(i));
    BOOST_CHECK_EQUAL(t->length(), 1000);
    BOOST_CHECK(!(t->contains("key0")));
    BOOST_CHECK(t->contains("key1"));
}

BOOST_AUTO_TEST_CASE(ChainSpecifics) {
    ChainHashTable t(4);
    BOOST_CHECK_EQUAL(t.capacity(), 4);
    BOOST_CHECK_EQUAL(t.type_name(), "ChainHashTable");
    for (const char* k : {"a", "b", "c", "d", "e"}) t.put(k, k);
    BOOST_CHECK(t.capacity() >= 8);
    BOOST_CHECK(t.longest_chain() >= 1);
    ChainHashTable zero(0);
    BOOST_CHECK_EQUAL(zero.capacity(), 8);
    ChainHashTable copy(t);
    copy.put("f", "f");
    BOOST_CHECK_EQUAL(t.length(), 5);
    ChainHashTable assigned;
    assigned = copy;
    ChainHashTable& self = assigned;
    assigned = self;
    BOOST_CHECK_EQUAL(assigned.length(), 6);
    ChainHashTable moved(std::move(assigned));
    BOOST_CHECK_EQUAL(moved.get("f").value_or("-"), "f");
    BOOST_CHECK(!(assigned.get("f").has_value()));
    BOOST_CHECK(!(assigned.remove("f")));
    BOOST_CHECK(assigned.put("x", "1"));
    ChainHashTable target;
    target = std::move(moved);
    BOOST_CHECK_EQUAL(target.length(), 6);
}

BOOST_AUTO_TEST_CASE(OpenAddressingTombstones) {
    OpenHashTable lin(OpenHashTable::Probing::Linear, 11);
    OpenHashTable dbl(OpenHashTable::Probing::Double, 11);
    BOOST_CHECK_EQUAL(lin.type_name(), "OpenHashTable/linear");
    BOOST_CHECK_EQUAL(dbl.type_name(), "OpenHashTable/double");
    BOOST_CHECK(lin.probing() == OpenHashTable::Probing::Linear);
    for (OpenHashTable* t : {&lin, &dbl}) {
        for (const char* k : {"pear", "lime", "grape"}) t->put(k, k);
        BOOST_CHECK_EQUAL(t->capacity(), 11);
        BOOST_CHECK(t->probes("grape") >= 2);
        BOOST_CHECK(t->remove("lime"));
        BOOST_CHECK_EQUAL(t->get("grape").value_or("-"), "grape");
        BOOST_CHECK(t->put("lime", "again"));
        BOOST_CHECK_EQUAL(t->get("lime").value_or("-"), "again");
        BOOST_CHECK(t->probes("missing") >= 1);
    }
    BOOST_CHECK_EQUAL(lin.probes("grape"), 3);
    BOOST_CHECK_EQUAL(dbl.probes("grape"), 2);
}

BOOST_AUTO_TEST_CASE(OpenHelpersAndCopies) {
    BOOST_CHECK(OpenHashTable::is_prime(2));
    BOOST_CHECK(OpenHashTable::is_prime(1009));
    BOOST_CHECK(!(OpenHashTable::is_prime(1)));
    BOOST_CHECK(!(OpenHashTable::is_prime(1000)));
    BOOST_CHECK_EQUAL(OpenHashTable::next_prime(1000), 1009);
    OpenHashTable small(OpenHashTable::Probing::Linear, 1);
    BOOST_CHECK_EQUAL(small.capacity(), 3);
    OpenHashTable t(OpenHashTable::Probing::Double, 11);
    for (int i = 0; i < 30; i++) {
        t.put("k" + std::to_string(i), "v");
        t.remove("k" + std::to_string(i));
    }
    BOOST_CHECK_EQUAL(t.length(), 0);
    BOOST_CHECK(t.put("alive", "1"));
    OpenHashTable copy(t);
    copy.put("other", "2");
    BOOST_CHECK_EQUAL(t.length(), 1);
    OpenHashTable assigned;
    assigned = copy;
    OpenHashTable& self = assigned;
    assigned = self;
    BOOST_CHECK_EQUAL(assigned.length(), 2);
    OpenHashTable moved(std::move(assigned));
    BOOST_CHECK_EQUAL(moved.get("other").value_or("-"), "2");
    BOOST_CHECK(!(assigned.get("other").has_value()));
    BOOST_CHECK(!(assigned.remove("other")));
    BOOST_CHECK_EQUAL(assigned.probes("other"), 0);
    BOOST_CHECK(assigned.put("x", "1"));
    OpenHashTable target;
    target = std::move(moved);
    BOOST_CHECK_EQUAL(target.length(), 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SerializationTests)

BOOST_DATA_TEST_CASE(Roundtrip, boost::unit_test::data::make(format_samples()), param) {
    auto original = param.sample.make_filled();
    std::stringstream buffer;
    original->save(buffer, param.format);
    auto loaded = param.sample.make_empty();
    loaded->load(buffer, param.format);
    BOOST_CHECK_EQUAL(loaded->type_name(), original->type_name());
    BOOST_CHECK_EQUAL(loaded->length(), original->length());
    BOOST_CHECK_EQUAL(describe(*loaded), describe(*original));
}

BOOST_DATA_TEST_CASE(EmptyContainer, boost::unit_test::data::make(samples()), param) {
    auto c = param.make_empty();
    BOOST_CHECK(c->empty());
    std::stringstream out;
    c->save(out, Format::Text);
    BOOST_CHECK_EQUAL(out.str(), "{\n  \"" + c->type_name() + "\": []\n}\n");
    c->load(out, Format::Text);
    BOOST_CHECK_EQUAL(c->length(), 0);
}

BOOST_DATA_TEST_CASE(RejectsBadJson, boost::unit_test::data::make(std::vector<std::string>{
    "", "[]", "{}", "{\"Array\": [\"\\ud800\"]}", "{\"Array\": [\"1\"", "{\"Array\": [\"1\" \"2\"]}",
    "{\"Array\" [\"1\"]}", "{\"Array\": [\"bad\\q\"]}", "{\"Array\": [\"\\u12\"]}",
    "{\"Array\": [\"open", "{\"Queue\": [\"x\"]}", "{\"Array\": [], \"Queue\": []}",
    "{\"Array\": []} tail"}), param) {
    Array a;
    a.push_back("old");
    std::stringstream in(param);
    BOOST_CHECK_THROW(a.load(in, Format::Text), std::runtime_error);
    BOOST_CHECK_EQUAL(join(a), "old");
}

BOOST_DATA_TEST_CASE(RejectsBadTree, boost::unit_test::data::make(std::vector<std::string>{
    "{\"RBTree\": [\"5(X)\"]}", "{\"RBTree\": [\"5\"]}", "{\"RBTree\": [\"5(R)\"]}",
    "{\"RBTree\": [\"5(B)\", \"5(R)\"]}", "{\"RBTree\": [\"5(B)\", \"3(B)\"]}"}), param) {
    RBTree t;
    std::stringstream good("{\"RBTree\": [\"5(B)\", \"3(R)\", \"8(R)\"]}");
    BOOST_CHECK_NO_THROW(t.load(good, Format::Text));
    std::stringstream in(param);
    BOOST_CHECK_THROW(t.load(in, Format::Text), std::runtime_error);
    BOOST_CHECK_EQUAL(join(t.preorder()), "5(B) 3(R) 8(R)");
}

BOOST_AUTO_TEST_CASE(TextFormatIsLab1Json) {
    SinglyList sl;
    for (const char* v : {"a", "b"}) sl.push_back(v);
    BOOST_CHECK_EQUAL(text_of(sl), "{\n  \"SinglyList\": [\"a\", \"b\"]\n}\n");
    RBTree t;
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) t.insert(v);
    BOOST_CHECK_EQUAL(text_of(t), "{\n  \"RBTree\": [\"20(B)\", \"10(B)\", \"5(R)\", \"15(R)\", \"30(B)\", \"25(R)\"]\n}\n");
    ChainHashTable h;
    h.put("k", "v");
    BOOST_CHECK_EQUAL(text_of(h), "{\n  \"ChainHashTable\": [\"k\", \"v\"]\n}\n");
    Stack st;
    for (const char* v : {"1", "2", "3"}) st.push(v);
    Stack st2;
    roundtrip(st, st2, Format::Text);
    std::string v;
    BOOST_CHECK(st2.pop(v));
    BOOST_CHECK_EQUAL(v, "3");
}

BOOST_AUTO_TEST_CASE(JsonParsingLikeLab1) {
    Array a;
    std::stringstream spaced(" {  \"Array\" :\n [ \"1\" ,\t\"2\" ] } ");
    BOOST_CHECK_NO_THROW(a.load(spaced, Format::Text));
    BOOST_CHECK_EQUAL(join(a), "1 2");
    std::stringstream numbers("{\"Array\": [15, -3]}");
    BOOST_CHECK_NO_THROW(a.load(numbers, Format::Text));
    BOOST_CHECK_EQUAL(join(a), "15 -3");
    std::stringstream escapes("{\"Array\": [\"a\\\"b\", \"c\\\\d\", \"e\\nf\", \"\\t\\r\\/\\b\\f\", \"\\u0041\\u00e9\\u20ac\"]}");
    BOOST_CHECK_NO_THROW(a.load(escapes, Format::Text));
    BOOST_CHECK_EQUAL(a.length(), 5);
    BOOST_CHECK_EQUAL(a.at(0), "a\"b");
    BOOST_CHECK_EQUAL(a.at(1), "c\\d");
    BOOST_CHECK_EQUAL(a.at(2), "e\nf");
    BOOST_CHECK_EQUAL(a.at(4), "A\xc3\xa9\xe2\x82\xac");
    Array ctrl;
    ctrl.push_back(std::string(1, '\x01'));
    Array ctrl2;
    roundtrip(ctrl, ctrl2, Format::Text);
    BOOST_CHECK_EQUAL(ctrl2.at(0), std::string(1, '\x01'));
}

BOOST_AUTO_TEST_CASE(FilesRoundtrip) {
    const std::string bin = "lab3_test_array.bin";
    const std::string txt = "lab3_test_array.json";
    Array a;
    for (const char* v : {"1", "2", "3"}) a.push_back(v);
    a.save_file(bin, Format::Binary);
    a.save_file(txt, Format::Text);
    Array b;
    b.load_file(bin, Format::Binary);
    BOOST_CHECK_EQUAL(join(b), "1 2 3");
    Array c;
    c.load_file(txt, Format::Text);
    BOOST_CHECK_EQUAL(join(c), "1 2 3");
    std::remove(bin.c_str());
    std::remove(txt.c_str());
    BOOST_CHECK_THROW(c.load_file("no/such/dir/file.bin", Format::Binary), std::runtime_error);
    BOOST_CHECK_THROW(c.save_file("no/such/dir/file.bin", Format::Binary), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(RejectsBadBinary) {
    Array a;
    a.push_back("x");
    std::stringstream bin;
    a.save(bin, Format::Binary);
    SinglyList wrong_type;
    BOOST_CHECK_THROW(wrong_type.load(bin, Format::Binary), std::runtime_error);
    std::stringstream garbage("XXXXrest");
    BOOST_CHECK_THROW(a.load(garbage, Format::Binary), std::runtime_error);
    std::string full;
    {
        std::stringstream b2;
        a.save(b2, Format::Binary);
        full = b2.str();
    }
    std::stringstream truncated(full.substr(0, full.size() - 1));
    BOOST_CHECK_THROW(a.load(truncated, Format::Binary), std::runtime_error);
    BOOST_CHECK_EQUAL(join(a), "x");
    std::stringstream short_header(full.substr(0, 6));
    BOOST_CHECK_THROW(a.load(short_header, Format::Binary), std::runtime_error);
    std::stringstream huge;
    huge.write("L3B1", 4);
    const char len_type[4] = {5, 0, 0, 0};
    huge.write(len_type, 4);
    huge.write("Array", 5);
    const char one[4] = {1, 0, 0, 0};
    huge.write(one, 4);
    const char big[4] = {0, 0, 0, 0x7f};
    huge.write(big, 4);
    BOOST_CHECK_THROW(a.load(huge, Format::Binary), std::runtime_error);
    RBTree t;
    t.insert("1");
    t.insert("2");
    std::stringstream tb;
    t.save(tb, Format::Binary);
    std::string data = tb.str();
    std::stringstream cut(data.substr(0, data.size() - 2));
    RBTree t2;
    BOOST_CHECK_THROW(t2.load(cut, Format::Binary), std::runtime_error);
    BOOST_CHECK(t2.empty());
    ChainHashTable h;
    std::stringstream odd("{\"ChainHashTable\": [\"key\"]}");
    BOOST_CHECK_THROW(h.load(odd, Format::Text), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
