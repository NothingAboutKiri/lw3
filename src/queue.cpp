#include "queue.hpp"

using namespace std;

void Queue::push(const string& value) {
    list.push_back(value);
}

bool Queue::pop(string& removed) {
    return list.pop_front(removed);
}

bool Queue::front(string& value) const {
    return list.front(value);
}

Array Queue::values() const {
    return list.values();
}
