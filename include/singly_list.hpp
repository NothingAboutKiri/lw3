#pragma once
#include <string>
#include "array.hpp"
#include "container.hpp"

using namespace std;

// Односвязный список с указателями на голову и хвост
class SinglyList : public Sequence {
    struct Node {
        string value;
        Node* next;
    };
    Node* head{nullptr};
    Node* tail{nullptr};
    int count{0};

public:
    SinglyList();
    ~SinglyList() override;
    SinglyList(const SinglyList&) = delete;
    SinglyList& operator=(const SinglyList&) = delete;
    SinglyList(SinglyList&&) = delete;
    SinglyList& operator=(SinglyList&&) = delete;

    void push_front(const string& value);   // в голову
    void push_back(const string& value);   // в хвост
    bool insert_after(const string& key, const string& value);  // после key
    bool insert_before(const string& key, const string& value); // перед key
    bool pop_front(string& removed);   // из головы
    bool pop_back(string& removed);   // из хвоста
    bool remove(const string& key);   // по значению
    bool get(int index, string& value) const;   // по индексу
    [[nodiscard]] bool contains(const string& key) const;   // поиск
    bool front(string& value) const;   // первый элемент
    [[nodiscard]] Array values() const override;   // от головы к хвосту

    // ---------- ЛР №3: методы иерархии Container / Sequence ----------
    [[nodiscard]] int length() const override { return count; }
    void clear() override;
    [[nodiscard]] string type_name() const override { return "SinglyList"; }

protected:
    void restore(const string& value) override { push_back(value); }
};
