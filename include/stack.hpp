#pragma once
#include "container.hpp"
#include "singly_list.hpp"

using namespace std;

// Стек на односвязном списке: вершина - голова списка
class Stack : public Sequence {
    SinglyList list;
public:
    void push(const string& value);
    bool pop(string& removed);
    bool top(string& value) const;
    [[nodiscard]] Array values() const override;   // от вершины ко дну

    // ---------- ЛР №3: методы иерархии Container / Sequence ----------
    [[nodiscard]] int length() const override { return list.length(); }
    void clear() override { list.clear(); }
    [[nodiscard]] string type_name() const override { return "Stack"; }

protected:
    // значения сохраняются от вершины ко дну, поэтому при загрузке каждое следующее кладётся под предыдущее
    void restore(const string& value) override { list.push_back(value); }
};
