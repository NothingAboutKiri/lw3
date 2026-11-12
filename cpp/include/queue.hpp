#pragma once
#include "container.hpp"
#include "singly_list.hpp"

using namespace std;

// Очередь на односвязном списке: добавление в хвост, удаление из головы
class Queue : public Sequence {
    SinglyList list;
public:
    void push(const string& value);
    bool pop(string& removed);
    bool front(string& value) const;
    [[nodiscard]] Array values() const override;   // от первого к последнему

    // ---------- ЛР №3: методы иерархии Container / Sequence ----------
    [[nodiscard]] int length() const override { return list.length(); }
    void clear() override { list.clear(); }
    [[nodiscard]] string type_name() const override { return "Queue"; }

protected:
    void restore(const string& value) override { push(value); }
};
