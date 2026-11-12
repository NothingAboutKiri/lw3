#include "singly_list.hpp"

using namespace std;

SinglyList::SinglyList() = default;

SinglyList::~SinglyList() {
    while (head != nullptr) {
        Node* old = head;
        head = head->next;
        delete old;
    }
}

void SinglyList::push_front(const string& value) {
    Node* node = new Node{value, head};
    head = node;
    if (tail == nullptr) {
        tail = node;
    }
    count++;
}

void SinglyList::push_back(const string& value) {
    Node* node = new Node{value, nullptr};
    if (tail == nullptr) {
        head = node;
    } else {
        tail->next = node;
    }
    tail = node;
    count++;
}

bool SinglyList::insert_after(const string& key, const string& value) {
    Node* current = head;
    while (current != nullptr && current->value != key) {
        current = current->next;
    }
    if (current == nullptr) {
        return false;
    }
    Node* node = new Node{value, current->next};
    current->next = node;
    if (current == tail) {
        tail = node;
    }
    count++;
    return true;
}

bool SinglyList::insert_before(const string& key, const string& value) {
    if (head == nullptr) {
        return false;
    }
    if (head->value == key) {
        push_front(value);
        return true;
    }
    Node* previous = head;   // ищем узел, стоящий перед key
    while (previous->next != nullptr && previous->next->value != key) {
        previous = previous->next;
    }
    if (previous->next == nullptr) {
        return false;
    }
    Node* node = new Node{value, previous->next};
    previous->next = node;
    count++;
    return true;
}

bool SinglyList::pop_front(string& removed) {
    if (head == nullptr) {
        return false;
    }
    Node* old = head;
    removed = old->value;
    head = head->next;
    if (head == nullptr) {
        tail = nullptr;
    }
    delete old;
    count--;
    return true;
}

bool SinglyList::pop_back(string& removed) {
    if (head == nullptr) {
        return false;
    }
    if (head == tail) {
        return pop_front(removed);
    }
    Node* previous = head;   // ищем предпоследний узел
    while (previous->next != tail) {
        previous = previous->next;
    }
    removed = tail->value;
    delete tail;
    tail = previous;
    tail->next = nullptr;
    count--;
    return true;
}

bool SinglyList::remove(const string& key) {
    if (head == nullptr) {
        return false;
    }
    if (head->value == key) {
        string removed;
        return pop_front(removed);
    }
    Node* previous = head;
    while (previous->next != nullptr && previous->next->value != key) {
        previous = previous->next;
    }
    if (previous->next == nullptr) {
        return false;
    }
    Node* old = previous->next;
    previous->next = old->next;
    if (old == tail) {
        tail = previous;
    }
    delete old;
    count--;
    return true;
}

bool SinglyList::get(int index, string& value) const {
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

bool SinglyList::contains(const string& key) const {
    for (Node* current = head; current != nullptr; current = current->next) {
        if (current->value == key) {
            return true;
        }
    }
    return false;
}

bool SinglyList::front(string& value) const {
    if (head == nullptr) {
        return false;
    }
    value = head->value;
    return true;
}

Array SinglyList::values() const {
    Array result;
    for (Node* current = head; current != nullptr; current = current->next) {
        result.push_back(current->value);
    }
    return result;
}

// ---------- ЛР №3 ----------

void SinglyList::clear() {
    string removed;
    while (pop_front(removed)) {
    }
}
