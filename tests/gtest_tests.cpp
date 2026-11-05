#include <gtest/gtest.h>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "helpers.hpp"

TEST(ArrayTests, PushBackAndGet) {
    Array a;
    EXPECT_TRUE(a.empty());
    for (const char* v : {"10", "20", "30", "40", "50"}) a.push_back(v);
    EXPECT_EQ(a.length(), 5);
    std::string v;
    EXPECT_TRUE(a.get(0, v));
    EXPECT_EQ(v, "10");
    EXPECT_EQ(a.at(4), "50");
    EXPECT_FALSE(a.get(5, v));
    EXPECT_FALSE(a.get(-1, v));
    EXPECT_FALSE(a.empty());
    EXPECT_EQ(a.type_name(), "Array");
}

TEST(ArrayTests, InsertAtBeginMiddleEnd) {
    Array a;
    EXPECT_TRUE(a.insert(0, "b"));
    EXPECT_TRUE(a.insert(0, "a"));
    EXPECT_TRUE(a.insert(2, "d"));
    EXPECT_TRUE(a.insert(2, "c"));
    EXPECT_EQ(join(a), "a b c d");
    EXPECT_FALSE(a.insert(-1, "x"));
    EXPECT_FALSE(a.insert(5, "x"));
    EXPECT_EQ(a.length(), 4);
}

TEST(ArrayTests, SetAndErase) {
    Array a;
    for (const char* v : {"1", "2", "3", "4"}) a.push_back(v);
    EXPECT_TRUE(a.set(0, "one"));
    EXPECT_FALSE(a.set(4, "z"));
    EXPECT_FALSE(a.set(-1, "z"));
    std::string removed;
    EXPECT_TRUE(a.erase(1, removed));
    EXPECT_EQ(removed, "2");
    EXPECT_EQ(join(a), "one 3 4");
    EXPECT_TRUE(a.erase(2, removed));
    EXPECT_EQ(removed, "4");
    EXPECT_FALSE(a.erase(2, removed));
    EXPECT_FALSE(a.erase(-1, removed));
    EXPECT_EQ(a.length(), 2);
}

TEST(ArrayTests, CopyMoveClearAndGrow) {
    Array a;
    for (int i = 0; i < 1000; i++) a.push_back(std::to_string(i));
    EXPECT_EQ(a.length(), 1000);
    EXPECT_EQ(a.at(999), "999");
    Array copy(a);
    copy.set(0, "changed");
    EXPECT_EQ(a.at(0), "0");
    Array assigned;
    assigned = copy;
    EXPECT_EQ(assigned.at(0), "changed");
    Array& self = assigned;
    assigned = self;
    EXPECT_EQ(assigned.length(), 1000);
    Array moved(std::move(copy));
    EXPECT_EQ(moved.at(0), "changed");
    copy.push_back("reused");
    EXPECT_EQ(join(copy), "reused");
    Array target;
    target = std::move(moved);
    EXPECT_EQ(target.length(), 1000);
    moved.push_back("again");
    EXPECT_EQ(moved.length(), 1);
    a.clear();
    EXPECT_TRUE(a.empty());
    a.push_back("again");
    EXPECT_EQ(join(a), "again");
    EXPECT_EQ(join(a.values()), "again");
}

TEST(SinglyListTests, PushFrontBack) {
    SinglyList l;
    std::string v;
    EXPECT_FALSE(l.front(v));
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    EXPECT_EQ(join(l), "a b c");
    EXPECT_TRUE(l.front(v));
    EXPECT_EQ(v, "a");
    EXPECT_EQ(l.length(), 3);
    EXPECT_EQ(l.type_name(), "SinglyList");
    SinglyList one;
    one.push_front("only");
    EXPECT_EQ(join(one), "only");
}

TEST(SinglyListTests, InsertAfterBefore) {
    SinglyList l;
    EXPECT_FALSE(l.insert_before("x", "y"));
    EXPECT_FALSE(l.insert_after("x", "y"));
    l.push_back("a");
    l.push_back("c");
    EXPECT_TRUE(l.insert_after("a", "b"));
    EXPECT_TRUE(l.insert_after("c", "d"));
    EXPECT_TRUE(l.insert_before("a", "start"));
    EXPECT_TRUE(l.insert_before("d", "c2"));
    EXPECT_FALSE(l.insert_after("missing", "x"));
    EXPECT_FALSE(l.insert_before("missing", "x"));
    EXPECT_EQ(join(l), "start a b c c2 d");
    l.push_back("tail");
    EXPECT_EQ(join(l), "start a b c c2 d tail");
}

TEST(SinglyListTests, PopFrontBack) {
    SinglyList l;
    for (const char* v : {"1", "2", "3"}) l.push_back(v);
    std::string v;
    EXPECT_TRUE(l.pop_front(v));
    EXPECT_EQ(v, "1");
    EXPECT_TRUE(l.pop_back(v));
    EXPECT_EQ(v, "3");
    EXPECT_TRUE(l.pop_back(v));
    EXPECT_EQ(v, "2");
    EXPECT_TRUE(l.empty());
    EXPECT_FALSE(l.pop_front(v));
    EXPECT_FALSE(l.pop_back(v));
    l.push_back("x");
    EXPECT_TRUE(l.pop_front(v));
    EXPECT_TRUE(l.empty());
    l.push_back("y");
    EXPECT_EQ(join(l), "y");
}

TEST(SinglyListTests, RemoveContainsGetClear) {
    SinglyList l;
    EXPECT_FALSE(l.remove("a"));
    for (const char* v : {"a", "b", "c", "d"}) l.push_back(v);
    EXPECT_TRUE(l.remove("a"));
    EXPECT_TRUE(l.remove("d"));
    EXPECT_TRUE(l.remove("c"));
    EXPECT_FALSE(l.remove("zzz"));
    EXPECT_TRUE(l.contains("b"));
    EXPECT_FALSE(l.contains("a"));
    std::string v;
    EXPECT_TRUE(l.get(0, v));
    EXPECT_EQ(v, "b");
    EXPECT_FALSE(l.get(1, v));
    EXPECT_FALSE(l.get(-1, v));
    l.push_back("e");
    EXPECT_EQ(join(l), "b e");
    l.clear();
    EXPECT_TRUE(l.empty());
    l.push_front("z");
    EXPECT_EQ(join(l), "z");
}

TEST(DoublyListTests, PushAndValues) {
    DoublyList l;
    l.push_back("b");
    l.push_front("a");
    l.push_back("c");
    EXPECT_EQ(join(l), "a b c");
    EXPECT_EQ(l.type_name(), "DoublyList");
    DoublyList front_only;
    front_only.push_front("2");
    front_only.push_front("1");
    EXPECT_EQ(join(front_only), "1 2");
}

TEST(DoublyListTests, InsertAfterBefore) {
    DoublyList l;
    EXPECT_FALSE(l.insert_after("x", "y"));
    EXPECT_FALSE(l.insert_before("x", "y"));
    l.push_back("b");
    EXPECT_TRUE(l.insert_before("b", "a"));
    EXPECT_TRUE(l.insert_after("b", "d"));
    EXPECT_TRUE(l.insert_after("b", "c"));
    EXPECT_TRUE(l.insert_before("d", "c2"));
    EXPECT_TRUE(l.insert_after("d", "e"));
    EXPECT_EQ(join(l), "a b c c2 d e");
    std::string v;
    EXPECT_TRUE(l.pop_back(v));
    EXPECT_EQ(v, "e");
    EXPECT_TRUE(l.pop_front(v));
    EXPECT_EQ(v, "a");
}

TEST(DoublyListTests, PopRemoveGet) {
    DoublyList l;
    std::string v;
    EXPECT_FALSE(l.pop_front(v));
    EXPECT_FALSE(l.pop_back(v));
    for (const char* s : {"1", "2", "3", "4", "5"}) l.push_back(s);
    EXPECT_TRUE(l.pop_front(v));
    EXPECT_EQ(v, "1");
    EXPECT_TRUE(l.pop_back(v));
    EXPECT_EQ(v, "5");
    EXPECT_TRUE(l.remove("3"));
    EXPECT_FALSE(l.remove("3"));
    EXPECT_TRUE(l.contains("2"));
    EXPECT_FALSE(l.contains("9"));
    EXPECT_TRUE(l.get(1, v));
    EXPECT_EQ(v, "4");
    EXPECT_FALSE(l.get(2, v));
    EXPECT_FALSE(l.get(-1, v));
    EXPECT_TRUE(l.remove("2"));
    EXPECT_TRUE(l.remove("4"));
    EXPECT_TRUE(l.empty());
    l.push_back("x");
    EXPECT_TRUE(l.pop_back(v));
    EXPECT_TRUE(l.empty());
    l.push_back("y");
    EXPECT_TRUE(l.pop_front(v));
    EXPECT_TRUE(l.empty());
}

TEST(DoublyListTests, Clear) {
    DoublyList l;
    for (const char* v : {"x", "y", "z"}) l.push_back(v);
    l.clear();
    EXPECT_TRUE(l.empty());
    EXPECT_EQ(join(l), "");
    l.push_back("again");
    EXPECT_EQ(l.length(), 1);
}

TEST(StackQueueTests, StackIsLifo) {
    Stack s;
    std::string v;
    EXPECT_FALSE(s.pop(v));
    EXPECT_FALSE(s.top(v));
    for (const char* x : {"1", "2", "3"}) s.push(x);
    EXPECT_TRUE(s.top(v));
    EXPECT_EQ(v, "3");
    EXPECT_EQ(join(s), "3 2 1");
    EXPECT_TRUE(s.pop(v));
    EXPECT_EQ(v, "3");
    EXPECT_TRUE(s.pop(v));
    EXPECT_EQ(v, "2");
    EXPECT_EQ(s.length(), 1);
    EXPECT_EQ(s.type_name(), "Stack");
    s.clear();
    EXPECT_TRUE(s.empty());
}

TEST(StackQueueTests, QueueIsFifo) {
    Queue q;
    std::string v;
    EXPECT_FALSE(q.pop(v));
    EXPECT_FALSE(q.front(v));
    for (const char* x : {"a", "b", "c"}) q.push(x);
    EXPECT_TRUE(q.front(v));
    EXPECT_EQ(v, "a");
    EXPECT_TRUE(q.pop(v));
    EXPECT_EQ(v, "a");
    q.push("d");
    EXPECT_EQ(join(q), "b c d");
    EXPECT_EQ(q.length(), 3);
    EXPECT_EQ(q.type_name(), "Queue");
    q.clear();
    EXPECT_TRUE(q.empty());
}

TEST(RBTreeTests, InsertBalancesLikeLab1) {
    RBTree t;
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) {
        EXPECT_TRUE(t.insert(v));
    }
    EXPECT_FALSE(t.insert("15"));
    EXPECT_EQ(join(t.preorder()), "20(B) 10(B) 5(R) 15(R) 30(B) 25(R)");
    EXPECT_EQ(join(t.inorder()), "5(R) 10(B) 15(R) 20(B) 25(R) 30(B)");
    EXPECT_EQ(join(t.postorder()), "5(R) 15(R) 10(B) 25(R) 30(B) 20(B)");
    EXPECT_EQ(join(t.bfs()), "20(B) 10(B) 30(B) 5(R) 15(R) 25(R)");
    EXPECT_EQ(join(t.keys()), "5 10 15 20 25 30");
    EXPECT_EQ(join(t), "5 10 15 20 25 30");
    EXPECT_TRUE(t.is_valid());
    EXPECT_EQ(t.length(), 6);
    EXPECT_EQ(t.type_name(), "RBTree");
}

TEST(RBTreeTests, SearchMinMaxNextPrev) {
    RBTree t;
    std::string v;
    EXPECT_FALSE(t.min_key(v));
    EXPECT_FALSE(t.max_key(v));
    for (const char* s : {"10", "20", "30", "15", "5", "25"}) t.insert(s);
    EXPECT_TRUE(t.contains("25"));
    EXPECT_FALSE(t.contains("99"));
    EXPECT_TRUE(t.min_key(v));
    EXPECT_EQ(v, "5");
    EXPECT_TRUE(t.max_key(v));
    EXPECT_EQ(v, "30");
    EXPECT_TRUE(t.next("15", v));
    EXPECT_EQ(v, "20");
    EXPECT_TRUE(t.next("20", v));
    EXPECT_EQ(v, "25");
    EXPECT_TRUE(t.prev("15", v));
    EXPECT_EQ(v, "10");
    EXPECT_TRUE(t.prev("20", v));
    EXPECT_EQ(v, "15");
    EXPECT_TRUE(t.prev("25", v));
    EXPECT_EQ(v, "20");
    EXPECT_FALSE(t.next("30", v));
    EXPECT_FALSE(t.prev("5", v));
    EXPECT_FALSE(t.next("99", v));
    EXPECT_FALSE(t.prev("99", v));
}

TEST(RBTreeTests, EraseAllCases) {
    RBTree t;
    EXPECT_FALSE(t.erase("1"));
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) t.insert(v);
    EXPECT_TRUE(t.erase("20"));
    EXPECT_EQ(join(t.preorder()), "25(B) 10(B) 5(R) 15(R) 30(B)");
    EXPECT_TRUE(t.erase("10"));
    EXPECT_EQ(join(t.preorder()), "25(B) 15(B) 5(R) 30(B)");
    EXPECT_FALSE(t.erase("99"));
    EXPECT_TRUE(t.erase("5"));
    EXPECT_TRUE(t.erase("30"));
    EXPECT_TRUE(t.erase("25"));
    EXPECT_TRUE(t.erase("15"));
    EXPECT_TRUE(t.empty());
    EXPECT_TRUE(t.is_valid());
}

TEST(RBTreeTests, StaysValidUnderRandomOps) {
    RBTree t;
    unsigned x = 12345;
    int inserted = 0;
    for (int i = 0; i < 3000; i++) {
        x = x * 1103515245U + 12345U;
        std::string key = std::to_string((x >> 8) % 500);
        if ((x >> 4) % 3 == 0) t.erase(key); else inserted += t.insert(key) ? 1 : 0;
    }
    EXPECT_TRUE(inserted > 0);
    EXPECT_TRUE(t.is_valid());
}

TEST(RBTreeTests, OrderingAndDiagram) {
    RBTree t;
    EXPECT_EQ(t.diagram(), "(empty tree)\n");
    EXPECT_EQ(join(t.bfs()), "");
    for (const char* v : {"2", "10", "-5", "abc", "99"}) t.insert(v);
    EXPECT_EQ(join(t.keys()), "-5 2 10 99 abc");
    RBTree d;
    for (const char* v : {"2", "1", "3", "4"}) d.insert(v);
    EXPECT_EQ(d.diagram(), "2(B)\n|-- L: 1(B)\n`-- R: 3(B)\n    `-- R: 4(R)\n");
    RBTree left;
    for (const char* v : {"3", "2", "1", "0"}) left.insert(v);
    EXPECT_EQ(left.diagram(), "2(B)\n|-- L: 1(B)\n|   `-- L: 0(R)\n`-- R: 3(B)\n");
    left.clear();
    EXPECT_TRUE(left.empty());
    EXPECT_TRUE(left.insert("z"));
    EXPECT_TRUE(left.contains("z"));
}

TEST(HashTableTests, HashFunction) {
    EXPECT_EQ(hash_string(""), 0U);
    EXPECT_EQ(hash_string("a"), 97U);
    EXPECT_EQ(hash_string("ab"), 97U * 31U + 98U);
    EXPECT_TRUE(hash_string("ab") != hash_string("ba"));
}

class HashTableTests_PutGetRemove : public ::testing::TestWithParam<TableKind> {};

TEST_P(HashTableTests_PutGetRemove, Run) {
    const TableKind& param = GetParam();
    auto t = param.make();
    EXPECT_TRUE(t->empty());
    EXPECT_FALSE(t->get("x").has_value());
    EXPECT_FALSE(t->remove("x"));
    EXPECT_TRUE(t->put("apple", "1"));
    EXPECT_TRUE(t->put("pear", "2"));
    EXPECT_FALSE(t->put("apple", "10"));
    EXPECT_EQ(t->get("apple").value_or("-"), "10");
    EXPECT_TRUE(t->contains("pear"));
    EXPECT_FALSE(t->contains("plum"));
    EXPECT_EQ(t->length(), 2);
    EXPECT_TRUE(t->load_factor() > 0.0);
    EXPECT_TRUE(t->remove("pear"));
    EXPECT_FALSE(t->remove("pear"));
    EXPECT_FALSE(t->contains("pear"));
    EXPECT_EQ(pairs(*t), "apple=10");
    t->clear();
    EXPECT_TRUE(t->empty());
    EXPECT_TRUE(t->put("after", "clear"));
}

INSTANTIATE_TEST_SUITE_P(HashTableTests, HashTableTests_PutGetRemove, ::testing::ValuesIn(table_kinds()));

class HashTableTests_GrowsAndKeepsAllKeys : public ::testing::TestWithParam<TableKind> {};

TEST_P(HashTableTests_GrowsAndKeepsAllKeys, Run) {
    const TableKind& param = GetParam();
    auto t = param.make();
    const int start = t->capacity();
    for (int i = 0; i < 2000; i++) t->put("key" + std::to_string(i), std::to_string(i));
    EXPECT_EQ(t->length(), 2000);
    EXPECT_TRUE(t->capacity() > start);
    EXPECT_TRUE(t->load_factor() <= 0.75);
    bool all = true;
    for (int i = 0; i < 2000; i++) all = all && t->get("key" + std::to_string(i)).value_or("") == std::to_string(i);
    EXPECT_TRUE(all);
    for (int i = 0; i < 2000; i += 2) t->remove("key" + std::to_string(i));
    EXPECT_EQ(t->length(), 1000);
    EXPECT_FALSE(t->contains("key0"));
    EXPECT_TRUE(t->contains("key1"));
}

INSTANTIATE_TEST_SUITE_P(HashTableTests, HashTableTests_GrowsAndKeepsAllKeys, ::testing::ValuesIn(table_kinds()));

TEST(HashTableTests, ChainSpecifics) {
    ChainHashTable t(4);
    EXPECT_EQ(t.capacity(), 4);
    EXPECT_EQ(t.type_name(), "ChainHashTable");
    for (const char* k : {"a", "b", "c", "d", "e"}) t.put(k, k);
    EXPECT_TRUE(t.capacity() >= 8);
    EXPECT_TRUE(t.longest_chain() >= 1);
    ChainHashTable zero(0);
    EXPECT_EQ(zero.capacity(), 8);
    ChainHashTable copy(t);
    copy.put("f", "f");
    EXPECT_EQ(t.length(), 5);
    ChainHashTable assigned;
    assigned = copy;
    ChainHashTable& self = assigned;
    assigned = self;
    EXPECT_EQ(assigned.length(), 6);
    ChainHashTable moved(std::move(assigned));
    EXPECT_EQ(moved.get("f").value_or("-"), "f");
    EXPECT_FALSE(assigned.get("f").has_value());
    EXPECT_FALSE(assigned.remove("f"));
    EXPECT_TRUE(assigned.put("x", "1"));
    ChainHashTable target;
    target = std::move(moved);
    EXPECT_EQ(target.length(), 6);
}

TEST(HashTableTests, OpenAddressingTombstones) {
    OpenHashTable lin(OpenHashTable::Probing::Linear, 11);
    OpenHashTable dbl(OpenHashTable::Probing::Double, 11);
    EXPECT_EQ(lin.type_name(), "OpenHashTable/linear");
    EXPECT_EQ(dbl.type_name(), "OpenHashTable/double");
    EXPECT_TRUE(lin.probing() == OpenHashTable::Probing::Linear);
    for (OpenHashTable* t : {&lin, &dbl}) {
        for (const char* k : {"pear", "lime", "grape"}) t->put(k, k);
        EXPECT_EQ(t->capacity(), 11);
        EXPECT_TRUE(t->probes("grape") >= 2);
        EXPECT_TRUE(t->remove("lime"));
        EXPECT_EQ(t->get("grape").value_or("-"), "grape");
        EXPECT_TRUE(t->put("lime", "again"));
        EXPECT_EQ(t->get("lime").value_or("-"), "again");
        EXPECT_TRUE(t->probes("missing") >= 1);
    }
    EXPECT_EQ(lin.probes("grape"), 3);
    EXPECT_EQ(dbl.probes("grape"), 2);
}

TEST(HashTableTests, OpenHelpersAndCopies) {
    EXPECT_TRUE(OpenHashTable::is_prime(2));
    EXPECT_TRUE(OpenHashTable::is_prime(1009));
    EXPECT_FALSE(OpenHashTable::is_prime(1));
    EXPECT_FALSE(OpenHashTable::is_prime(1000));
    EXPECT_EQ(OpenHashTable::next_prime(1000), 1009);
    OpenHashTable small(OpenHashTable::Probing::Linear, 1);
    EXPECT_EQ(small.capacity(), 3);
    OpenHashTable t(OpenHashTable::Probing::Double, 11);
    for (int i = 0; i < 30; i++) {
        t.put("k" + std::to_string(i), "v");
        t.remove("k" + std::to_string(i));
    }
    EXPECT_EQ(t.length(), 0);
    EXPECT_TRUE(t.put("alive", "1"));
    OpenHashTable copy(t);
    copy.put("other", "2");
    EXPECT_EQ(t.length(), 1);
    OpenHashTable assigned;
    assigned = copy;
    OpenHashTable& self = assigned;
    assigned = self;
    EXPECT_EQ(assigned.length(), 2);
    OpenHashTable moved(std::move(assigned));
    EXPECT_EQ(moved.get("other").value_or("-"), "2");
    EXPECT_FALSE(assigned.get("other").has_value());
    EXPECT_FALSE(assigned.remove("other"));
    EXPECT_EQ(assigned.probes("other"), 0);
    EXPECT_TRUE(assigned.put("x", "1"));
    OpenHashTable target;
    target = std::move(moved);
    EXPECT_EQ(target.length(), 2);
}

class SerializationTests_Roundtrip : public ::testing::TestWithParam<FormatSample> {};

TEST_P(SerializationTests_Roundtrip, Run) {
    const FormatSample& param = GetParam();
    auto original = param.sample.make_filled();
    std::stringstream buffer;
    original->save(buffer, param.format);
    auto loaded = param.sample.make_empty();
    loaded->load(buffer, param.format);
    EXPECT_EQ(loaded->type_name(), original->type_name());
    EXPECT_EQ(loaded->length(), original->length());
    EXPECT_EQ(describe(*loaded), describe(*original));
}

INSTANTIATE_TEST_SUITE_P(SerializationTests, SerializationTests_Roundtrip, ::testing::ValuesIn(format_samples()));

class SerializationTests_EmptyContainer : public ::testing::TestWithParam<Sample> {};

TEST_P(SerializationTests_EmptyContainer, Run) {
    const Sample& param = GetParam();
    auto c = param.make_empty();
    EXPECT_TRUE(c->empty());
    std::stringstream out;
    c->save(out, Format::Text);
    EXPECT_EQ(out.str(), "{\n  \"" + c->type_name() + "\": []\n}\n");
    c->load(out, Format::Text);
    EXPECT_EQ(c->length(), 0);
}

INSTANTIATE_TEST_SUITE_P(SerializationTests, SerializationTests_EmptyContainer, ::testing::ValuesIn(samples()));

class SerializationTests_RejectsBadJson : public ::testing::TestWithParam<std::string> {};

TEST_P(SerializationTests_RejectsBadJson, Run) {
    const std::string& param = GetParam();
    Array a;
    a.push_back("old");
    std::stringstream in(param);
    EXPECT_THROW(a.load(in, Format::Text), std::runtime_error);
    EXPECT_EQ(join(a), "old");
}

INSTANTIATE_TEST_SUITE_P(SerializationTests, SerializationTests_RejectsBadJson, ::testing::ValuesIn(std::vector<std::string>{
    "", "[]", "{}", "{\"Array\": [\"\\ud800\"]}", "{\"Array\": [\"1\"", "{\"Array\": [\"1\" \"2\"]}",
    "{\"Array\" [\"1\"]}", "{\"Array\": [\"bad\\q\"]}", "{\"Array\": [\"\\u12\"]}",
    "{\"Array\": [\"open", "{\"Queue\": [\"x\"]}", "{\"Array\": [], \"Queue\": []}",
    "{\"Array\": []} tail"}));

class SerializationTests_RejectsBadTree : public ::testing::TestWithParam<std::string> {};

TEST_P(SerializationTests_RejectsBadTree, Run) {
    const std::string& param = GetParam();
    RBTree t;
    std::stringstream good("{\"RBTree\": [\"5(B)\", \"3(R)\", \"8(R)\"]}");
    EXPECT_NO_THROW(t.load(good, Format::Text));
    std::stringstream in(param);
    EXPECT_THROW(t.load(in, Format::Text), std::runtime_error);
    EXPECT_EQ(join(t.preorder()), "5(B) 3(R) 8(R)");
}

INSTANTIATE_TEST_SUITE_P(SerializationTests, SerializationTests_RejectsBadTree, ::testing::ValuesIn(std::vector<std::string>{
    "{\"RBTree\": [\"5(X)\"]}", "{\"RBTree\": [\"5\"]}", "{\"RBTree\": [\"5(R)\"]}",
    "{\"RBTree\": [\"5(B)\", \"5(R)\"]}", "{\"RBTree\": [\"5(B)\", \"3(B)\"]}"}));

TEST(SerializationTests, TextFormatIsLab1Json) {
    SinglyList sl;
    for (const char* v : {"a", "b"}) sl.push_back(v);
    EXPECT_EQ(text_of(sl), "{\n  \"SinglyList\": [\"a\", \"b\"]\n}\n");
    RBTree t;
    for (const char* v : {"10", "20", "30", "15", "5", "25"}) t.insert(v);
    EXPECT_EQ(text_of(t), "{\n  \"RBTree\": [\"20(B)\", \"10(B)\", \"5(R)\", \"15(R)\", \"30(B)\", \"25(R)\"]\n}\n");
    ChainHashTable h;
    h.put("k", "v");
    EXPECT_EQ(text_of(h), "{\n  \"ChainHashTable\": [\"k\", \"v\"]\n}\n");
    Stack st;
    for (const char* v : {"1", "2", "3"}) st.push(v);
    Stack st2;
    roundtrip(st, st2, Format::Text);
    std::string v;
    EXPECT_TRUE(st2.pop(v));
    EXPECT_EQ(v, "3");
}

TEST(SerializationTests, JsonParsingLikeLab1) {
    Array a;
    std::stringstream spaced(" {  \"Array\" :\n [ \"1\" ,\t\"2\" ] } ");
    EXPECT_NO_THROW(a.load(spaced, Format::Text));
    EXPECT_EQ(join(a), "1 2");
    std::stringstream numbers("{\"Array\": [15, -3]}");
    EXPECT_NO_THROW(a.load(numbers, Format::Text));
    EXPECT_EQ(join(a), "15 -3");
    std::stringstream escapes("{\"Array\": [\"a\\\"b\", \"c\\\\d\", \"e\\nf\", \"\\t\\r\\/\\b\\f\", \"\\u0041\\u00e9\\u20ac\"]}");
    EXPECT_NO_THROW(a.load(escapes, Format::Text));
    EXPECT_EQ(a.length(), 5);
    EXPECT_EQ(a.at(0), "a\"b");
    EXPECT_EQ(a.at(1), "c\\d");
    EXPECT_EQ(a.at(2), "e\nf");
    EXPECT_EQ(a.at(4), "A\xc3\xa9\xe2\x82\xac");
    Array ctrl;
    ctrl.push_back(std::string(1, '\x01'));
    Array ctrl2;
    roundtrip(ctrl, ctrl2, Format::Text);
    EXPECT_EQ(ctrl2.at(0), std::string(1, '\x01'));
}

TEST(SerializationTests, FilesRoundtrip) {
    const std::string bin = "lab3_test_array.bin";
    const std::string txt = "lab3_test_array.json";
    Array a;
    for (const char* v : {"1", "2", "3"}) a.push_back(v);
    a.save_file(bin, Format::Binary);
    a.save_file(txt, Format::Text);
    Array b;
    b.load_file(bin, Format::Binary);
    EXPECT_EQ(join(b), "1 2 3");
    Array c;
    c.load_file(txt, Format::Text);
    EXPECT_EQ(join(c), "1 2 3");
    std::remove(bin.c_str());
    std::remove(txt.c_str());
    EXPECT_THROW(c.load_file("no/such/dir/file.bin", Format::Binary), std::runtime_error);
    EXPECT_THROW(c.save_file("no/such/dir/file.bin", Format::Binary), std::runtime_error);
}

TEST(SerializationTests, RejectsBadBinary) {
    Array a;
    a.push_back("x");
    std::stringstream bin;
    a.save(bin, Format::Binary);
    SinglyList wrong_type;
    EXPECT_THROW(wrong_type.load(bin, Format::Binary), std::runtime_error);
    std::stringstream garbage("XXXXrest");
    EXPECT_THROW(a.load(garbage, Format::Binary), std::runtime_error);
    std::string full;
    {
        std::stringstream b2;
        a.save(b2, Format::Binary);
        full = b2.str();
    }
    std::stringstream truncated(full.substr(0, full.size() - 1));
    EXPECT_THROW(a.load(truncated, Format::Binary), std::runtime_error);
    EXPECT_EQ(join(a), "x");
    std::stringstream short_header(full.substr(0, 6));
    EXPECT_THROW(a.load(short_header, Format::Binary), std::runtime_error);
    std::stringstream huge;
    huge.write("L3B1", 4);
    const char len_type[4] = {5, 0, 0, 0};
    huge.write(len_type, 4);
    huge.write("Array", 5);
    const char one[4] = {1, 0, 0, 0};
    huge.write(one, 4);
    const char big[4] = {0, 0, 0, 0x7f};
    huge.write(big, 4);
    EXPECT_THROW(a.load(huge, Format::Binary), std::runtime_error);
    RBTree t;
    t.insert("1");
    t.insert("2");
    std::stringstream tb;
    t.save(tb, Format::Binary);
    std::string data = tb.str();
    std::stringstream cut(data.substr(0, data.size() - 2));
    RBTree t2;
    EXPECT_THROW(t2.load(cut, Format::Binary), std::runtime_error);
    EXPECT_TRUE(t2.empty());
    ChainHashTable h;
    std::stringstream odd("{\"ChainHashTable\": [\"key\"]}");
    EXPECT_THROW(h.load(odd, Format::Text), std::runtime_error);
}
