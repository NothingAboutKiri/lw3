#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include "container.hpp"

// Полиномиальная хеш-функция строки: h = h * 31 + код символа (mod 2^64)
std::uint64_t hash_string(const std::string& text);

// Абстрактная хеш-таблица «ключ -> значение»
class HashTable : public Container {
public:
    using PairVisitor = std::function<void(const std::string&, const std::string&)>;

    // Вставка или замена значения; возвращает true, если ключ новый
    virtual bool put(const std::string& key, const std::string& value) = 0;
    [[nodiscard]] virtual std::optional<std::string> get(const std::string& key) const = 0;
    virtual bool remove(const std::string& key) = 0;
    [[nodiscard]] bool contains(const std::string& key) const { return get(key).has_value(); }
    virtual void for_each_pair(const PairVisitor& visit) const = 0;
    [[nodiscard]] virtual int capacity() const = 0;
    [[nodiscard]] double load_factor() const { return static_cast<double>(length()) / capacity(); }

    // Сериализация пар одинакова для любых таблиц, поэтому написана в базовом классе
    void save_binary(std::ostream& out) const override;
    void load_binary(std::istream& in) override;
    void save_text(std::ostream& out) const override;
    void load_text(std::istream& in) override;
};

// Метод цепочек: ячейка - односвязный список пар, при заполнении > 0,75 таблица удваивается
class ChainHashTable : public HashTable {
public:
    explicit ChainHashTable(int buckets = START_BUCKETS);
    ChainHashTable(const ChainHashTable& other);
    ChainHashTable(ChainHashTable&& other) noexcept;
    ChainHashTable& operator=(const ChainHashTable& other);
    ChainHashTable& operator=(ChainHashTable&& other) noexcept;
    ~ChainHashTable() override;

    bool put(const std::string& key, const std::string& value) override;
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const override;
    bool remove(const std::string& key) override;
    void for_each_pair(const PairVisitor& visit) const override;
    [[nodiscard]] int capacity() const override { return bucket_count_; }
    [[nodiscard]] int longest_chain() const;

    [[nodiscard]] int length() const override { return count_; }
    void clear() override;
    [[nodiscard]] std::string type_name() const override { return "ChainHashTable"; }

private:
    static constexpr int START_BUCKETS = 8;
    struct Entry {
        std::string key;
        std::string value;
        Entry* next;
    };
    Entry** buckets_;
    int bucket_count_;
    int count_ = 0;

    [[nodiscard]] int index(const std::string& key) const;
    [[nodiscard]] Entry* find(const std::string& key) const;
    void rehash(int new_count);
    void insert_new(const std::string& key, const std::string& value);
    void free_buckets();
};

// Открытая адресация: линейное апробирование или двойное хеширование.
// Удалённая ячейка помечается «надгробием», чтобы не обрывать поиск.
class OpenHashTable : public HashTable {
public:
    enum class Probing : std::uint8_t { Linear, Double };

    explicit OpenHashTable(Probing probing = Probing::Double, int size = 11);
    OpenHashTable(const OpenHashTable& other);
    OpenHashTable(OpenHashTable&& other) noexcept;
    OpenHashTable& operator=(const OpenHashTable& other);
    OpenHashTable& operator=(OpenHashTable&& other) noexcept;
    ~OpenHashTable() override;

    bool put(const std::string& key, const std::string& value) override;
    [[nodiscard]] std::optional<std::string> get(const std::string& key) const override;
    bool remove(const std::string& key) override;
    void for_each_pair(const PairVisitor& visit) const override;
    [[nodiscard]] int capacity() const override { return capacity_; }
    [[nodiscard]] Probing probing() const { return probing_; }
    // Число проб при поиске ключа (для анализа методов)
    [[nodiscard]] int probes(const std::string& key) const;

    [[nodiscard]] int length() const override { return count_; }
    void clear() override;
    [[nodiscard]] std::string type_name() const override;

    static bool is_prime(int n);
    static int next_prime(int n);

private:
    enum class Cell : unsigned char { Empty, Busy, Deleted };
    Probing probing_;
    int capacity_ = 0;
    std::string* keys_ = nullptr;
    std::string* values_ = nullptr;
    Cell* cells_ = nullptr;
    int count_ = 0;
    int deleted_ = 0;

    // i-я проба: (h1 + i) mod m или (h1 + i * h2) mod m, h2 = 1 + H mod (m - 1)
    [[nodiscard]] int probe(std::uint64_t hash, int i) const;
    // Номер ячейки ключа или -1; probes - сколько ячеек просмотрено
    int find(const std::string& key, int& probes) const;
    void rebuild(int new_size);
    void insert_new(const std::string& key, const std::string& value);
    void allocate(int size);
    void release();
};
