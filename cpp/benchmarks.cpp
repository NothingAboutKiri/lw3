#include <benchmark/benchmark.h>
#include <sstream>
#include <string>
#include <vector>
#include "array.hpp"
#include "doubly_list.hpp"
#include "hash_table.hpp"
#include "queue.hpp"
#include "rb_tree.hpp"
#include "singly_list.hpp"
#include "stack.hpp"

namespace {

std::vector<std::string> make_keys(int n) {
    std::vector<std::string> keys;
    keys.reserve(static_cast<std::size_t>(n));
    unsigned x = 2463534242U;
    for (int i = 0; i < n; i++) {
        x ^= x << 13U;
        x ^= x >> 17U;
        x ^= x << 5U;
        keys.push_back(std::to_string(x % 1000000U));
    }
    return keys;
}

void BM_ArrayPushBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        Array a;
        for (const auto& k : keys) a.push_back(k);
        benchmark::DoNotOptimize(a.length());
    }
    state.SetComplexityN(state.range(0));
}

void BM_ArrayInsertFront(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        Array a;
        for (const auto& k : keys) a.insert(0, k);
        benchmark::DoNotOptimize(a.length());
    }
    state.SetComplexityN(state.range(0));
}

void BM_SinglyListPushFront(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        SinglyList l;
        for (const auto& k : keys) l.push_front(k);
        benchmark::DoNotOptimize(l.length());
    }
    state.SetComplexityN(state.range(0));
}

void BM_SinglyListContains(benchmark::State& state) {
    SinglyList l;
    for (const auto& k : make_keys(static_cast<int>(state.range(0)))) l.push_back(k);
    for (auto _ : state) {
        benchmark::DoNotOptimize(l.contains("missing"));
    }
    state.SetComplexityN(state.range(0));
}

void BM_DoublyListPushPopBack(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        DoublyList l;
        for (const auto& k : keys) l.push_back(k);
        std::string out;
        while (l.pop_back(out)) benchmark::DoNotOptimize(out);
    }
    state.SetComplexityN(state.range(0));
}

void BM_StackPushPop(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        Stack s;
        for (const auto& k : keys) s.push(k);
        std::string out;
        while (s.pop(out)) benchmark::DoNotOptimize(out);
    }
    state.SetComplexityN(state.range(0));
}

void BM_QueuePushPop(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        Queue q;
        for (const auto& k : keys) q.push(k);
        std::string out;
        while (q.pop(out)) benchmark::DoNotOptimize(out);
    }
    state.SetComplexityN(state.range(0));
}

template <typename Make>
void table_put(benchmark::State& state, Make make) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        auto t = make();
        for (const auto& k : keys) t.put(k, k);
        benchmark::DoNotOptimize(t.length());
    }
    state.SetComplexityN(state.range(0));
}

template <typename Make>
void table_get(benchmark::State& state, Make make) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    auto t = make();
    for (const auto& k : keys) t.put(k, k);
    std::size_t i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(t.get(keys[i++ % keys.size()]));
    }
    state.SetComplexityN(state.range(0));
}

ChainHashTable make_chain() { return ChainHashTable(); }
OpenHashTable make_linear() { return OpenHashTable(OpenHashTable::Probing::Linear); }
OpenHashTable make_double() { return OpenHashTable(OpenHashTable::Probing::Double); }

void BM_ChainPut(benchmark::State& state) { table_put(state, make_chain); }
void BM_LinearPut(benchmark::State& state) { table_put(state, make_linear); }
void BM_DoublePut(benchmark::State& state) { table_put(state, make_double); }
void BM_ChainGet(benchmark::State& state) { table_get(state, make_chain); }
void BM_LinearGet(benchmark::State& state) { table_get(state, make_linear); }
void BM_DoubleGet(benchmark::State& state) { table_get(state, make_double); }

void BM_RBTreeInsert(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    for (auto _ : state) {
        RBTree t;
        for (const auto& k : keys) t.insert(k);
        benchmark::DoNotOptimize(t.length());
    }
    state.SetComplexityN(state.range(0));
}

void BM_RBTreeContains(benchmark::State& state) {
    const auto keys = make_keys(static_cast<int>(state.range(0)));
    RBTree t;
    for (const auto& k : keys) t.insert(k);
    std::size_t i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(t.contains(keys[i++ % keys.size()]));
    }
    state.SetComplexityN(state.range(0));
}

void BM_Serialize(benchmark::State& state, Format format) {
    Array a;
    for (const auto& k : make_keys(static_cast<int>(state.range(0)))) a.push_back(k);
    for (auto _ : state) {
        std::stringstream buffer;
        a.save(buffer, format);
        Array b;
        b.load(buffer, format);
        benchmark::DoNotOptimize(b.length());
    }
    state.SetComplexityN(state.range(0));
}

}

BENCHMARK(BM_ArrayPushBack)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_ArrayInsertFront)->RangeMultiplier(10)->Range(100, 10000)->Complexity();
BENCHMARK(BM_SinglyListPushFront)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_SinglyListContains)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_DoublyListPushPopBack)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_StackPushPop)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_QueuePushPop)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_ChainPut)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_LinearPut)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_DoublePut)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_ChainGet)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_LinearGet)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_DoubleGet)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_RBTreeInsert)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK(BM_RBTreeContains)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK_CAPTURE(BM_Serialize, binary, Format::Binary)->RangeMultiplier(10)->Range(100, 100000)->Complexity();
BENCHMARK_CAPTURE(BM_Serialize, text, Format::Text)->RangeMultiplier(10)->Range(100, 100000)->Complexity();

BENCHMARK_MAIN();
