#include "doubly_list.hpp"

using namespace std;

DoublyList::DoublyList() = default;

DoublyList::~DoublyList() {
    while (head != nullptr) {
        Node* old = head;
        head = head->next;
        delete old;
    }
}

DoublyList::Node* DoublyList::find(const string& key) const {
    for (Node* current = head; current != nullptr; current = current->next) {
        if (current->value == key) {
            return current;
        }
    }
    return nullptr;
}

void DoublyList::unlink(Node* node) {
    if (node->prev != nullptr) {
        node->prev->next = node->next;
    } else {
        head = node->next;
    }
    if (node->next != nullptr) {
        node->next->prev = node->prev;
    } else {
        tail = node->prev;
    }
    delete node;
    count--;
}

void DoublyList::push_front(const string& value) {
    Node* node = new Node{value, nullptr, head};
    if (head != nullptr) {
        head->prev = node;
    } else {
        tail = node;
    }
    head = node;
    count++;
}

void DoublyList::push_back(const string& value) {
    Node* node = new Node{value, tail, nullptr};
    if (tail != nullptr) {
        tail->next = node;
    } else {
        head = node;
    }
    tail = node;
    count++;
}

bool DoublyList::insert_after(const string& key, const string& value) {
    Node* current = find(key);
    if (current == nullptr) {
        return false;
    }
    if (current == tail) {
        push_back(value);
        return true;
    }
    Node* node = new Node{value, current, current->next};
    current->next->prev = node;
    current->next = node;
    count++;
    return true;
}

bool DoublyList::insert_before(const string& key, const string& value) {
    Node* current = find(key);
    if (current == nullptr) {
        return false;
    }
    if (current == head) {
        push_front(value);
        return true;
    }
    Node* node = new Node{value, current->prev, current};
    current->prev->next = node;
    current->prev = node;
    count++;
    return true;
}

bool DoublyList::pop_front(string& removed) {
    if (head == nullptr) {
        return false;
    }
    removed = head->value;
    unlink(head);
    return true;
}

bool DoublyList::pop_back(string& removed) {
    if (tail == nullptr) {
        return false;
    }
    removed = tail->value;
    unlink(tail);
    return true;
}

bool DoublyList::remove(const string& key) {
    Node* node = find(key);
    if (node == nullptr) {
        return false;
    }
    unlink(node);
    return true;
}

bool DoublyList::get(int index, string& value) const {
    if (index < 0 || index >= count) {
        return false;
    }
    Node* current = head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    value = current->value;
    return true;
}

bool DoublyList::contains(const string& key) const {
    return find(key) != nullptr;
}

Array DoublyList::values() const {
    Array result;
    for (Node* current = head; current != nullptr; current = current->next) {
        result.push_back(current->value);
    }
    return result;
}

// ---------- ЛР №3 ----------

void DoublyList::clear() {
    while (head != nullptr) {
        Node* old = head;
        head = head->next;
        delete old;
    }
    tail = nullptr;
    count = 0;
}
