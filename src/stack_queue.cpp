#include <stdexcept>
#include "array.hpp"
#include "queue.hpp"
#include "stack.hpp"

std::string Stack::pop() {
    if (list_.empty()) {
        throw std::underflow_error("stack is empty");
    }
    return list_.pop_front();
}

const std::string& Stack::top() const {
    if (list_.empty()) {
        throw std::underflow_error("stack is empty");
    }
    return list_.front();
}

void Stack::for_each(const Visitor& visit) const {
    Array values;
    list_.for_each([&values](const std::string& value) { values.push_back(value); });
    for (int i = values.size(); i > 0; --i) {
        visit(values.get(i - 1));
    }
}

std::string Queue::pop() {
    if (list_.empty()) {
        throw std::underflow_error("queue is empty");
    }
    return list_.pop_front();
}

const std::string& Queue::front() const {
    if (list_.empty()) {
        throw std::underflow_error("queue is empty");
    }
    return list_.front();
}
