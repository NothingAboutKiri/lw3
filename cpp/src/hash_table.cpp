#include "hash_table.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include "array.hpp"
#include "codec.hpp"

std::uint64_t hash_string(const std::string& text) {
    std::uint64_t h = 0;
    for (const unsigned char c : text) {
        h = h * 31 + c;   // переполнение = взятие остатка по модулю 2^64
    }
    return h;
}

// ---------- HashTable: сериализация пар ----------

void HashTable::save_binary(std::ostream& out) const {
    codec::write_binary_header(out, type_name(), static_cast<std::uint32_t>(length()));
    for_each_pair([&out](const std::string& key, const std::string& value) {
        codec::write_string(out, key);
        codec::write_string(out, value);
    });
}

void HashTable::load_binary(std::istream& in) {
    const std::uint32_t count = codec::read_binary_header(in, type_name());
    clear();
    for (std::uint32_t i = 0; i < count; i++) {
        const std::string key = codec::read_string(in);
        put(key, codec::read_string(in));
    }
}

// Текстовый формат - JSON из ЛР №1: {"тип": ["ключ1", "значение1", "ключ2", "значение2", ...]}
void HashTable::save_text(std::ostream& out) const {
    Array flat;
    for_each_pair([&flat](const std::string& key, const std::string& value) {
        flat.push_back(key);
        flat.push_back(value);
    });
    codec::write_json(out, type_name(), flat);
}

void HashTable::load_text(std::istream& in) {
    const Array flat = codec::read_json(in, type_name());
    if (flat.length() % 2 != 0) {
        throw std::runtime_error("expected pairs key, value");
    }
    clear();
    for (int i = 0; i < flat.length(); i += 2) {
        put(flat.at(i), flat.at(i + 1));
    }
}

// ---------- ChainHashTable ----------

ChainHashTable::ChainHashTable(int buckets)
    : buckets_(new Entry*[buckets > 0 ? buckets : START_BUCKETS]()), bucket_count_(buckets > 0 ? buckets : START_BUCKETS) {}

ChainHashTable::ChainHashTable(const ChainHashTable& other)
    : HashTable(other), buckets_(new Entry*[other.bucket_count_]()), bucket_count_(other.bucket_count_) {
    other.for_each_pair([this](const std::string& key, const std::string& value) { insert_new(key, value); });
}

ChainHashTable::ChainHashTable(ChainHashTable&& other) noexcept
    : buckets_(std::exchange(other.buckets_, nullptr)), bucket_count_(std::exchange(other.bucket_count_, 0)),
      count_(std::exchange(other.count_, 0)) {}

ChainHashTable& ChainHashTable::operator=(const ChainHashTable& other) {
    if (this != &other) {
        ChainHashTable copy(other);
        *this = std::move(copy);
    }
    return *this;
}

ChainHashTable& ChainHashTable::operator=(ChainHashTable&& other) noexcept {
    std::swap(buckets_, other.buckets_);
    std::swap(bucket_count_, other.bucket_count_);
    std::swap(count_, other.count_);
    return *this;
}

ChainHashTable::~ChainHashTable() {
    free_buckets();
}

void ChainHashTable::free_buckets() {
    for (int i = 0; i < bucket_count_; i++) {
        Entry* e = buckets_[i];
        while (e != nullptr) {
            Entry* next = e->next;
            delete e;
            e = next;
        }
    }
    delete[] buckets_;
    buckets_ = nullptr;
}

int ChainHashTable::index(const std::string& key) const {
    return static_cast<int>(hash_string(key) % static_cast<std::uint64_t>(bucket_count_));
}

ChainHashTable::Entry* ChainHashTable::find(const std::string& key) const {
    for (Entry* e = buckets_[index(key)]; e != nullptr; e = e->next) {
        if (e->key == key) {
            return e;
        }
    }
    return nullptr;
}

// Перехеширование: все пары переносятся в таблицу нового размера (номер ячейки зависит от m)
void ChainHashTable::rehash(int new_count) {
    Entry** old = buckets_;
    const int old_count = bucket_count_;
    buckets_ = new Entry*[new_count]();
    bucket_count_ = new_count;
    for (int i = 0; i < old_count; i++) {
        Entry* e = old[i];
        while (e != nullptr) {
            Entry* next = e->next;
            const int j = index(e->key);
            e->next = buckets_[j];
            buckets_[j] = e;
            e = next;
        }
    }
    delete[] old;
}

bool ChainHashTable::put(const std::string& key, const std::string& value) {
    if (buckets_ == nullptr) {
        buckets_ = new Entry*[START_BUCKETS]();
        bucket_count_ = START_BUCKETS;
    }
    if (Entry* e = find(key)) {
        e->value = value;
        return false;
    }
    if ((count_ + 1) * 4 > bucket_count_ * 3) {   // коэффициент заполнения больше 0,75
        rehash(bucket_count_ * 2);
    }
    insert_new(key, value);
    return true;
}

void ChainHashTable::insert_new(const std::string& key, const std::string& value) {
    const int i = index(key);
    buckets_[i] = new Entry{key, value, buckets_[i]};
    count_++;
}

std::optional<std::string> ChainHashTable::get(const std::string& key) const {
    if (buckets_ == nullptr) {
        return std::nullopt;
    }
    if (Entry* e = find(key)) {
        return e->value;
    }
    return std::nullopt;
}

bool ChainHashTable::remove(const std::string& key) {
    if (buckets_ == nullptr) {
        return false;
    }
    Entry** link = &buckets_[index(key)];
    while (*link != nullptr) {
        if ((*link)->key == key) {
            Entry* old = *link;
            *link = old->next;
            delete old;
            count_--;
            return true;
        }
        link = &(*link)->next;
    }
    return false;
}

void ChainHashTable::for_each_pair(const PairVisitor& visit) const {
    for (int i = 0; i < bucket_count_; i++) {
        for (Entry* e = buckets_[i]; e != nullptr; e = e->next) {
            visit(e->key, e->value);
        }
    }
}

int ChainHashTable::longest_chain() const {
    int best = 0;
    for (int i = 0; i < bucket_count_; i++) {
        int len = 0;
        for (Entry* e = buckets_[i]; e != nullptr; e = e->next) {
            len++;
        }
        best = len > best ? len : best;
    }
    return best;
}

void ChainHashTable::clear() {
    free_buckets();
    buckets_ = new Entry*[START_BUCKETS]();
    bucket_count_ = START_BUCKETS;
    count_ = 0;
}

// ---------- OpenHashTable ----------

bool OpenHashTable::is_prime(int n) {
    if (n < 2) {
        return false;
    }
    for (int d = 2; d * d <= n; d++) {
        if (n % d == 0) {
            return false;
        }
    }
    return true;
}

int OpenHashTable::next_prime(int n) {
    while (!is_prime(n)) {
        n++;
    }
    return n;
}

void OpenHashTable::allocate(int size) {
    capacity_ = next_prime(size < 3 ? 3 : size);   // простое m: любой шаг h2 обходит все ячейки
    keys_ = new std::string[capacity_];
    values_ = new std::string[capacity_];
    cells_ = new Cell[capacity_];
    for (int i = 0; i < capacity_; i++) {
        cells_[i] = Cell::Empty;
    }
    count_ = 0;
    deleted_ = 0;
}

void OpenHashTable::release() {
    delete[] keys_;
    delete[] values_;
    delete[] cells_;
    keys_ = nullptr;
    values_ = nullptr;
    cells_ = nullptr;
}

OpenHashTable::OpenHashTable(Probing probing, int size)
    : probing_(probing) {
    allocate(size);
}

OpenHashTable::OpenHashTable(const OpenHashTable& other)
    : HashTable(other), probing_(other.probing_) {
    allocate(other.capacity_);
    other.for_each_pair([this](const std::string& key, const std::string& value) { insert_new(key, value); });
}

OpenHashTable::OpenHashTable(OpenHashTable&& other) noexcept
    : probing_(other.probing_), capacity_(std::exchange(other.capacity_, 0)),
      keys_(std::exchange(other.keys_, nullptr)), values_(std::exchange(other.values_, nullptr)),
      cells_(std::exchange(other.cells_, nullptr)), count_(std::exchange(other.count_, 0)),
      deleted_(std::exchange(other.deleted_, 0)) {}

OpenHashTable& OpenHashTable::operator=(const OpenHashTable& other) {
    if (this != &other) {
        OpenHashTable copy(other);
        *this = std::move(copy);
    }
    return *this;
}

OpenHashTable& OpenHashTable::operator=(OpenHashTable&& other) noexcept {
    std::swap(probing_, other.probing_);
    std::swap(capacity_, other.capacity_);
    std::swap(keys_, other.keys_);
    std::swap(values_, other.values_);
    std::swap(cells_, other.cells_);
    std::swap(count_, other.count_);
    std::swap(deleted_, other.deleted_);
    return *this;
}

OpenHashTable::~OpenHashTable() {
    release();
}

std::string OpenHashTable::type_name() const {
    return probing_ == Probing::Linear ? "OpenHashTable/linear" : "OpenHashTable/double";
}

int OpenHashTable::probe(std::uint64_t hash, int i) const {
    auto m = static_cast<std::uint64_t>(capacity_);
    const std::uint64_t h1 = hash % m;
    const std::uint64_t step = probing_ == Probing::Linear ? 1 : 1 + (hash % (m - 1));
    return static_cast<int>((h1 + static_cast<std::uint64_t>(i) * step) % m);
}

int OpenHashTable::find(const std::string& key, int& probes) const {
    const std::uint64_t h = hash_string(key);
    probes = 0;
    for (int i = 0; i < capacity_; i++) {
        const int pos = probe(h, i);
        probes = i + 1;
        if (cells_[pos] == Cell::Empty) {
            return -1;
        }
        if (cells_[pos] == Cell::Busy && keys_[pos] == key) {
            return pos;
        }
    }
    return -1;
}

void OpenHashTable::rebuild(int new_size) {
    std::string* old_keys = keys_;
    std::string* old_values = values_;
    Cell* old_cells = cells_;
    const int old_capacity = capacity_;
    allocate(new_size);
    for (int i = 0; i < old_capacity; i++) {
        if (old_cells[i] == Cell::Busy) {
            insert_new(old_keys[i], old_values[i]);
        }
    }
    delete[] old_keys;
    delete[] old_values;
    delete[] old_cells;
}

bool OpenHashTable::put(const std::string& key, const std::string& value) {
    if (cells_ == nullptr) {
        allocate(11);
    }
    int probes = 0;
    const int pos = find(key, probes);
    if (pos >= 0) {
        values_[pos] = value;
        return false;
    }
    if ((count_ + deleted_ + 1) * 4 > capacity_ * 3) {   // заполнено больше 75 % (с надгробиями)
        rebuild((count_ + 1) * 2 > capacity_ ? capacity_ * 2 : capacity_);
    }
    insert_new(key, value);
    return true;
}

void OpenHashTable::insert_new(const std::string& key, const std::string& value) {
    const std::uint64_t h = hash_string(key);
    for (int i = 0; i < capacity_; i++) {
        const int pos = probe(h, i);
        if (cells_[pos] != Cell::Busy) {
            if (cells_[pos] == Cell::Deleted) {
                deleted_--;
            }
            keys_[pos] = key;
            values_[pos] = value;
            cells_[pos] = Cell::Busy;
            count_++;
            return;
        }
    }
    throw std::logic_error("hash table is full");
}

std::optional<std::string> OpenHashTable::get(const std::string& key) const {
    if (cells_ == nullptr) {
        return std::nullopt;
    }
    int probes = 0;
    const int pos = find(key, probes);
    if (pos < 0) {
        return std::nullopt;
    }
    return values_[pos];
}

int OpenHashTable::probes(const std::string& key) const {
    int probes = 0;
    if (cells_ != nullptr) {
        find(key, probes);
    }
    return probes;
}

bool OpenHashTable::remove(const std::string& key) {
    if (cells_ == nullptr) {
        return false;
    }
    int probes = 0;
    const int pos = find(key, probes);
    if (pos < 0) {
        return false;
    }
    cells_[pos] = Cell::Deleted;   // надгробие: поиск продолжается через эту ячейку
    keys_[pos].clear();
    values_[pos].clear();
    count_--;
    deleted_++;
    return true;
}

void OpenHashTable::for_each_pair(const PairVisitor& visit) const {
    for (int i = 0; i < capacity_; i++) {
        if (cells_[i] == Cell::Busy) {
            visit(keys_[i], values_[i]);
        }
    }
}

void OpenHashTable::clear() {
    release();
    allocate(11);
}
