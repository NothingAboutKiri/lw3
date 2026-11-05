#pragma once
#include <string>
#include "container.hpp"

using namespace std;

// Самописный динамический массив строк.
// Любое значение хранится как строка, поэтому в массив можно
// положить и числа (10, -5), и слова (a, b, abc).
class Array : public Sequence {
    string* data;   // буфер с элементами
    int size{0};   // сколько элементов занято
    int capacity{4};   // сколько мест выделено

    void resize();  // увеличить буфер в два раза

public:
    Array();
    ~Array() override;
    Array(const Array& other);   // копия массива
    Array& operator=(const Array& other);   // присваивание с копированием
    Array(Array&& other) noexcept;   // перенос буфера без копирования
    Array& operator=(Array&& other) noexcept;

    void push_back(const string& value);   // добавить в конец
    bool insert(int index, const string& value);   // вставить по индексу
    bool get(int index, string& value) const;   // прочитать по индексу
    bool set(int index, const string& value);   // заменить по индексу
    bool erase(int index, string& removed);   // удалить по индексу
    [[nodiscard]] int length() const override;   // длина массива
    [[nodiscard]] string at(int index) const;   // элемент без проверки (для циклов)

    // ---------- ЛР №3: методы иерархии Container / Sequence ----------
    void clear() override;
    [[nodiscard]] string type_name() const override { return "Array"; }
    [[nodiscard]] Array values() const override { return *this; }

protected:
    void restore(const string& value) override { push_back(value); }
};
