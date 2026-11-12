#pragma once
#include <string>
#include "array.hpp"
#include "container.hpp"

using namespace std;

// Двусвязный список с указателями head и tail
class DoublyList : public Sequence {
    struct Node {
        string value;
        Node* prev;
        Node* next;
    };
    Node* head{nullptr};
    Node* tail{nullptr};
    int count{0};

    [[nodiscard]] Node* find(const string& key) const;   // поиск узла по значению
    void unlink(Node* node);   // исключить узел из списка и удалить

public:
    DoublyList();
    ~DoublyList() override;
    DoublyList(const DoublyList&) = delete;
    DoublyList& operator=(const DoublyList&) = delete;
    DoublyList(DoublyList&&) = delete;
    DoublyList& operator=(DoublyList&&) = delete;

    void push_front(const string& value);
    void push_back(const string& value);
    bool insert_after(const string& key, const string& value);
    bool insert_before(const string& key, const string& value);
    bool pop_front(string& removed);
    bool pop_back(string& removed);
    bool remove(const string& key);
    bool get(int index, string& value) const;
    [[nodiscard]] bool contains(const string& key) const;
    [[nodiscard]] Array values() const override;

    // ---------- ЛР №3: методы иерархии Container / Sequence ----------
    [[nodiscard]] int length() const override { return count; }
    void clear() override;
    [[nodiscard]] string type_name() const override { return "DoublyList"; }

protected:
    void restore(const string& value) override { push_back(value); }
};
