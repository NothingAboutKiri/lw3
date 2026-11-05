#include "array.hpp"

using namespace std;

Array::Array() : data(new string[4]) {}

Array::~Array() {
    delete[] data;
}

Array::Array(const Array& other)
    : data(new string[other.capacity]), size(other.size), capacity(other.capacity) {
    for (int i = 0; i < size; i++) {
        data[i] = other.data[i];
    }
}

Array& Array::operator=(const Array& other) {
    if (this == &other) {
        return *this;
    }
    delete[] data;
    capacity = other.capacity;
    size = other.size;
    data = new string[capacity];
    for (int i = 0; i < size; i++) {
        data[i] = other.data[i];
    }
    return *this;
}

// Перенос: буфер забирается у other, other остаётся пустым массивом без буфера
Array::Array(Array&& other) noexcept : data(other.data), size(other.size), capacity(other.capacity) {
    other.data = nullptr;
    other.size = 0;
    other.capacity = 0;
}

Array& Array::operator=(Array&& other) noexcept {
    if (this != &other) {
        delete[] data;
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }
    return *this;
}

void Array::resize() {
    const int newCapacity = capacity > 0 ? capacity * 2 : 4;   // после переноса буфера нет
    auto* newData = new string[newCapacity];
    for (int i = 0; i < size; i++) {
        newData[i] = data[i];
    }
    delete[] data;
    data = newData;
    capacity = newCapacity;
}

void Array::push_back(const string& value) {
    if (size == capacity) {
        resize();
    }
    data[size] = value;
    size++;
}

bool Array::insert(int index, const string& value) {
    if (index < 0 || index > size) {
        return false;
    }
    if (size == capacity) {
        resize();
    }
    for (int i = size; i > index; i--) {   // сдвиг вправо
        data[i] = data[i - 1];
    }
    data[index] = value;
    size++;
    return true;
}

bool Array::get(int index, string& value) const {
    if (index < 0 || index >= size) {
        return false;
    }
    value = data[index];
    return true;
}

bool Array::set(int index, const string& value) {
    if (index < 0 || index >= size) {
        return false;
    }
    data[index] = value;
    return true;
}

bool Array::erase(int index, string& removed) {
    if (index < 0 || index >= size) {
        return false;
    }
    removed = data[index];
    for (int i = index; i < size - 1; i++) {   // сдвиг влево
        data[i] = data[i + 1];
    }
    size--;
    return true;
}

int Array::length() const {
    return size;
}

string Array::at(int index) const {
    return data[index];
}

// ---------- ЛР №3 ----------

void Array::clear() {
    size = 0;
}
