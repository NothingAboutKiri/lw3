#!/usr/bin/env bash
# Сборка с покрытием, запуск трёх наборов тестов и HTML-отчёт gcovr одним файлом coverage/index.html
set -euo pipefail
cd "$(dirname "$0")/.."
if ! command -v gcovr >/dev/null; then
    echo 'Нужен gcovr: установите его перед построением HTML-отчёта.' >&2
    exit 1
fi
cmake -S . -B build-cov -DLAB3_COVERAGE=ON -DLAB3_TESTS=ON -DLAB3_BENCH=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cov -j
rm -rf coverage
mkdir -p coverage
find build-cov -name '*.gcda' -delete
for t in tests_boost tests_gtest tests_catch2; do ./build-cov/$t > /dev/null; done
gcovr -r . build-cov/CMakeFiles/lab3_core.dir/src \
    build-cov/CMakeFiles/tests_boost.dir/tests \
    build-cov/CMakeFiles/tests_gtest.dir/tests \
    build-cov/CMakeFiles/tests_catch2.dir/tests \
    --filter src/ --filter include/ --html --html-title 'ЛР3: покрытие C++' -o coverage/index.html --print-summary
