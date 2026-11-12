#include "stack.hpp"

using namespace std;

void Stack::push(const string& value) {
    list.push_front(value);
}

bool Stack::pop(string& removed) {
    return list.pop_front(removed);
}

bool Stack::top(string& value) const {
    return list.front(value);
}

Array Stack::values() const {
    return list.values();
}
