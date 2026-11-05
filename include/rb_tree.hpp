#pragma once
#include <string>
#include <utility>
#include "array.hpp"
#include "container.hpp"

using namespace std;

// Красно-чёрное дерево поиска
class RBTree : public Sequence {
    struct Node {
        Node(string k, bool is_red, Node* l, Node* r, Node* p)
            : key(std::move(k)), red(is_red), left(l), right(r), parent(p) {}
        string key;
        bool red;   // true - красный, false - чёрный
        Node* left;
        Node* right;
        Node* parent;
    };
    Node* nil;   // общий чёрный лист вместо пустых указателей
    Node* root;
    int count{0};

    static bool less(const string& a, const string& b);  // сравнение ключей
    [[nodiscard]] Node* search(const string& key) const;
    Node* minimum(Node* x) const;
    Node* maximum(Node* x) const;
    void left_rotate(Node* x);
    void right_rotate(Node* x);
    void insert_fix(Node* z);
    void transplant(Node* u, Node* v);
    void erase_fix(Node* x);
    void clear(Node* x);
    static string label(Node* x);   // ключ с цветом, например 10(B)
    void preorder(Node* x, Array& out) const;
    void inorder(Node* x, Array& out) const;
    void postorder(Node* x, Array& out) const;
    void collect_keys(Node* x, Array& out) const;
    void draw(Node* x, const string& prefix, const string& edge, bool last, string& out) const;
    int check(Node* x, const string* low, const string* high, int& seen) const;
    void attach(const string& key, bool red);   // ЛР №3: узел на своё место без балансировки
    void load_labels(const Array& labels);   // ЛР №3: восстановить дерево по меткам 20(B)

public:
    RBTree();
    ~RBTree() override;
    RBTree(const RBTree&) = delete;
    RBTree& operator=(const RBTree&) = delete;
    RBTree(RBTree&&) = delete;
    RBTree& operator=(RBTree&&) = delete;

    bool insert(const string& key);   // false, если ключ уже есть
    bool erase(const string& key);   // false, если ключа нет
    [[nodiscard]] bool contains(const string& key) const;
    bool min_key(string& key) const;
    bool max_key(string& key) const;
    bool next(const string& key, string& result) const;   // следующий по возрастанию
    bool prev(const string& key, string& result) const;   // предыдущий
    [[nodiscard]] Array keys() const;   // ключи по возрастанию (для файла)
    [[nodiscard]] Array preorder() const;   // обходы с цветами
    [[nodiscard]] Array inorder() const;
    [[nodiscard]] Array postorder() const;
    [[nodiscard]] Array bfs() const;
    [[nodiscard]] string diagram() const;   // рисунок дерева
    [[nodiscard]] bool is_valid() const;   // проверка свойств

    // ---------- ЛР №3: методы иерархии Container / Sequence ----------
    [[nodiscard]] int length() const override { return count; }
    void clear() override;
    [[nodiscard]] string type_name() const override { return "RBTree"; }
    [[nodiscard]] Array values() const override { return keys(); }
    // дерево сохраняется прямым обходом с цветами (метки 20(B)), чтобы восстановить точную форму
    void save_binary(ostream& out) const override;
    void load_binary(istream& in) override;
    void save_text(ostream& out) const override;
    void load_text(istream& in) override;

protected:
    void restore(const string& value) override { insert(value); }
};
