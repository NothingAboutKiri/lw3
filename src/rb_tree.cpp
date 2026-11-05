#include "rb_tree.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include "codec.hpp"

using namespace std;

namespace {

// Целое число: необязательный минус и от 1 до 18 цифр
bool is_number(const string& s) {
    int start = 0;
    if (!s.empty() && s[0] == '-') {
        start = 1;
    }
    const int digits = (int)s.size() - start;
    if (digits < 1 || digits > 18) {
        return false;
    }
    for (int i = start; i < (int)s.size(); i++) {
        if (s[i] < '0' || s[i] > '9') {
            return false;
        }
    }
    return true;
}

}  // namespace

// Числа сравниваются как числа и стоят раньше остальных строк
bool RBTree::less(const string& a, const string& b) {
    const bool aNum = is_number(a);
    const bool bNum = is_number(b);
    if (aNum && bNum) {
        const long long x = stoll(a);
        const long long y = stoll(b);
        if (x != y) {
            return x < y;
        }
        return a < b;   // "1" и "01": одно число, разные записи
    }
    if (aNum != bNum) {
        return aNum;
    }
    return a < b;
}

RBTree::RBTree() : nil(new Node{"", false, nullptr, nullptr, nullptr}), root(nil) {
    nil->left = nil;
    nil->right = nil;
    nil->parent = nil;
}

RBTree::~RBTree() {
    clear(root);
    delete nil;
}

void RBTree::clear(Node* x) {
    if (x == nil) {
        return;
    }
    clear(x->left);
    clear(x->right);
    delete x;
}

RBTree::Node* RBTree::search(const string& key) const {
    Node* x = root;
    while (x != nil && x->key != key) {
        if (less(key, x->key)) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    return x;
}

RBTree::Node* RBTree::minimum(Node* x) const {
    while (x->left != nil) {
        x = x->left;
    }
    return x;
}

RBTree::Node* RBTree::maximum(Node* x) const {
    while (x->right != nil) {
        x = x->right;
    }
    return x;
}

void RBTree::left_rotate(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil) {
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void RBTree::right_rotate(Node* x) {
    Node* y = x->left;
    x->left = y->right;
    if (y->right != nil) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil) {
        root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

bool RBTree::insert(const string& key) {
    Node* parent = nil;
    Node* x = root;
    while (x != nil) {           // спуск как в обычном дереве поиска
        parent = x;
        if (key == x->key) {
            return false;
        }
        if (less(key, x->key)) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    Node* z = new Node{key, true, nil, nil, parent};   // новый узел красный
    if (parent == nil) {
        root = z;
    } else if (less(key, parent->key)) {
        parent->left = z;
    } else {
        parent->right = z;
    }
    insert_fix(z);
    count++;
    return true;
}

void RBTree::insert_fix(Node* z) {
    while (z->parent->red) {
        Node* grand = z->parent->parent;
        if (z->parent == grand->left) {
            Node* uncle = grand->right;
            if (uncle->red) {                    // дядя красный: перекраска
                z->parent->red = false;
                uncle->red = false;
                grand->red = true;
                z = grand;
            } else {
                if (z == z->parent->right) {     // "угол": поворот у родителя
                    z = z->parent;
                    left_rotate(z);
                }
                z->parent->red = false;          // "цепочка": поворот у деда
                z->parent->parent->red = true;
                right_rotate(z->parent->parent);
            }
        } else {                                 // зеркальные случаи
            Node* uncle = grand->left;
            if (uncle->red) {
                z->parent->red = false;
                uncle->red = false;
                grand->red = true;
                z = grand;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(z);
                }
                z->parent->red = false;
                z->parent->parent->red = true;
                left_rotate(z->parent->parent);
            }
        }
    }
    root->red = false;
}

void RBTree::transplant(Node* u, Node* v) {
    if (u->parent == nil) {
        root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

bool RBTree::erase(const string& key) {
    Node* z = search(key);
    if (z == nil) {
        return false;
    }
    Node* y = z;               // узел, который реально уходит из дерева
    bool yWasRed = y->red;
    Node* x = nullptr;         // узел, встающий на место y
    if (z->left == nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == nil) {
        x = z->left;
        transplant(z, z->left);
    } else {                   // два потомка: берём минимум правого поддерева
        y = minimum(z->right);
        yWasRed = y->red;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->red = z->red;
    }
    delete z;
    count--;
    if (!yWasRed) {            // удалён чёрный узел - нужна балансировка
        erase_fix(x);
    }
    return true;
}

void RBTree::erase_fix(Node* x) {
    while (x != root && !x->red) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;          // брат
            if (w->red) {                        // случай 1: брат красный
                w->red = false;
                x->parent->red = true;
                left_rotate(x->parent);
                w = x->parent->right;
            }
            if (!w->left->red && !w->right->red) {   // случай 2: дети брата чёрные
                w->red = true;
                x = x->parent;
            } else {
                if (!w->right->red) {            // случай 3: красный ближний ребёнок
                    w->left->red = false;
                    w->red = true;
                    right_rotate(w);
                    w = x->parent->right;
                }
                w->red = x->parent->red;         // случай 4: красный дальний ребёнок
                x->parent->red = false;
                w->right->red = false;
                left_rotate(x->parent);
                x = root;
            }
        } else {                                 // зеркальные случаи
            Node* w = x->parent->left;
            if (w->red) {
                w->red = false;
                x->parent->red = true;
                right_rotate(x->parent);
                w = x->parent->left;
            }
            if (!w->right->red && !w->left->red) {
                w->red = true;
                x = x->parent;
            } else {
                if (!w->left->red) {
                    w->right->red = false;
                    w->red = true;
                    left_rotate(w);
                    w = x->parent->left;
                }
                w->red = x->parent->red;
                x->parent->red = false;
                w->left->red = false;
                right_rotate(x->parent);
                x = root;
            }
        }
    }
    x->red = false;
}

bool RBTree::contains(const string& key) const {
    return search(key) != nil;
}

bool RBTree::min_key(string& key) const {
    if (root == nil) {
        return false;
    }
    key = minimum(root)->key;
    return true;
}

bool RBTree::max_key(string& key) const {
    if (root == nil) {
        return false;
    }
    key = maximum(root)->key;
    return true;
}

bool RBTree::next(const string& key, string& result) const {
    Node* x = search(key);
    if (x == nil) {
        return false;
    }
    if (x->right != nil) {
        result = minimum(x->right)->key;
        return true;
    }
    Node* p = x->parent;            // поднимаемся, пока идём справа
    while (p != nil && x == p->right) {
        x = p;
        p = p->parent;
    }
    if (p == nil) {
        return false;
    }
    result = p->key;
    return true;
}

bool RBTree::prev(const string& key, string& result) const {
    Node* x = search(key);
    if (x == nil) {
        return false;
    }
    if (x->left != nil) {
        result = maximum(x->left)->key;
        return true;
    }
    Node* p = x->parent;
    while (p != nil && x == p->left) {
        x = p;
        p = p->parent;
    }
    if (p == nil) {
        return false;
    }
    result = p->key;
    return true;
}

string RBTree::label(Node* x) {
    if (x->red) {
        return x->key + "(R)";
    }
    return x->key + "(B)";
}

void RBTree::preorder(Node* x, Array& out) const {
    if (x == nil) {
        return;
    }
    out.push_back(label(x));
    preorder(x->left, out);
    preorder(x->right, out);
}

void RBTree::inorder(Node* x, Array& out) const {
    if (x == nil) {
        return;
    }
    inorder(x->left, out);
    out.push_back(label(x));
    inorder(x->right, out);
}

void RBTree::postorder(Node* x, Array& out) const {
    if (x == nil) {
        return;
    }
    postorder(x->left, out);
    postorder(x->right, out);
    out.push_back(label(x));
}

Array RBTree::preorder() const {
    Array out;
    preorder(root, out);
    return out;
}

Array RBTree::inorder() const {
    Array out;
    inorder(root, out);
    return out;
}

Array RBTree::postorder() const {
    Array out;
    postorder(root, out);
    return out;
}

Array RBTree::bfs() const {
    Array out;
    if (root == nil) {
        return out;
    }
    Node** level = new Node*[count];   // массив узлов используется как очередь
    int head = 0;
    int tail = 0;
    level[tail++] = root;
    while (head < tail) {
        Node* x = level[head++];
        out.push_back(label(x));
        if (x->left != nil) {
            level[tail++] = x->left;
        }
        if (x->right != nil) {
            level[tail++] = x->right;
        }
    }
    delete[] level;
    return out;
}

void RBTree::collect_keys(Node* x, Array& out) const {
    if (x == nil) {
        return;
    }
    collect_keys(x->left, out);
    out.push_back(x->key);
    collect_keys(x->right, out);
}

Array RBTree::keys() const {
    Array out;
    collect_keys(root, out);
    return out;
}

void RBTree::draw(Node* x, const string& prefix, const string& edge, bool last, string& out) const {
    if (x == nil) {
        return;
    }
    out += prefix + (last ? "`-- " : "|-- ") + edge + label(x) + "\n";
    const string childPrefix = prefix + (last ? "    " : "|   ");
    if (x->left != nil) {
        draw(x->left, childPrefix, "L: ", x->right == nil, out);
    }
    if (x->right != nil) {
        draw(x->right, childPrefix, "R: ", true, out);
    }
}

string RBTree::diagram() const {
    if (root == nil) {
        return "(empty tree)\n";
    }
    string out = label(root) + "\n";
    if (root->left != nil) {
        draw(root->left, "", "L: ", root->right == nil, out);
    }
    if (root->right != nil) {
        draw(root->right, "", "R: ", true, out);
    }
    return out;
}

// Возвращает чёрную высоту поддерева или -1, если свойства нарушены
int RBTree::check(Node* x, const string* low, const string* high, int& seen) const {
    if (x == nil) {
        return 1;
    }
    seen++;
    if (low != nullptr && !less(*low, x->key)) {
        return -1;                       // нарушен порядок ключей
    }
    if (high != nullptr && !less(x->key, *high)) {
        return -1;
    }
    if (x->left != nil && x->left->parent != x) {
        return -1;                       // неверная ссылка на родителя
    }
    if (x->right != nil && x->right->parent != x) {
        return -1;
    }
    if (x->red && (x->left->red || x->right->red)) {
        return -1;                       // два красных подряд
    }
    const int leftHeight = check(x->left, low, &x->key, seen);
    const int rightHeight = check(x->right, &x->key, high, seen);
    if (leftHeight < 0 || rightHeight < 0 || leftHeight != rightHeight) {
        return -1;                       // разная чёрная высота
    }
    return leftHeight + (x->red ? 0 : 1);
}

bool RBTree::is_valid() const {
    if (nil->red || root->red) {
        return false;
    }
    int seen = 0;
    return check(root, nullptr, nullptr, seen) > 0 && seen == count;
}

// ---------- ЛР №3: очистка и сериализация ----------

void RBTree::clear() {
    clear(root);
    root = nil;
    count = 0;
}

void RBTree::attach(const string& key, bool red) {
    Node* parent = nil;
    Node* x = root;
    while (x != nil) {
        parent = x;
        if (key == x->key) {
            throw runtime_error("duplicate key in tree data");
        }
        x = less(key, x->key) ? x->left : x->right;
    }
    Node* z = new Node{key, red, nil, nil, parent};
    if (parent == nil) {
        root = z;
    } else if (less(key, parent->key)) {
        parent->left = z;
    } else {
        parent->right = z;
    }
    count++;
}

void RBTree::load_labels(const Array& labels) {
    RBTree loaded;   // при ошибке временное дерево удалится, а текущее не изменится
    for (int i = 0; i < labels.length(); i++) {
        const string label = labels.at(i);
        if (label.size() < 4 || label[label.size() - 3] != '(' || label.back() != ')' ||
            (label[label.size() - 2] != 'R' && label[label.size() - 2] != 'B')) {
            throw runtime_error("expected key(R) or key(B)");
        }
        loaded.attach(label.substr(0, label.size() - 3), label[label.size() - 2] == 'R');
    }
    if (!loaded.is_valid()) {
        throw runtime_error("loaded data is not a valid red-black tree");
    }
    std::swap(nil, loaded.nil);
    std::swap(root, loaded.root);
    std::swap(count, loaded.count);
}

void RBTree::save_binary(ostream& out) const {
    const Array labels = preorder();
    codec::write_binary_header(out, type_name(), static_cast<uint32_t>(labels.length()));
    for (int i = 0; i < labels.length(); i++) {
        codec::write_string(out, labels.at(i));
    }
}

void RBTree::load_binary(istream& in) {
    const uint32_t n = codec::read_binary_header(in, type_name());
    Array labels;
    for (uint32_t i = 0; i < n; i++) {
        labels.push_back(codec::read_string(in));
    }
    load_labels(labels);
}

void RBTree::save_text(ostream& out) const {
    codec::write_json(out, type_name(), preorder());
}

void RBTree::load_text(istream& in) {
    load_labels(codec::read_json(in, type_name()));
}
