#!/usr/bin/env bash
# Покрытие встроенных и интеграционных тестов Rust без cargo-llvm-cov.
set -euo pipefail
cd "$(dirname "$0")/.."

llvm_profdata=$(command -v llvm-profdata || true)
llvm_cov=$(command -v llvm-cov || true)
if [[ -z "$llvm_profdata" || -z "$llvm_cov" ]]; then
    for version in 21 20 19 18; do
        if [[ -x "/usr/lib/llvm-$version/bin/llvm-profdata" && -x "/usr/lib/llvm-$version/bin/llvm-cov" ]]; then
            llvm_profdata="/usr/lib/llvm-$version/bin/llvm-profdata"
            llvm_cov="/usr/lib/llvm-$version/bin/llvm-cov"
            break
        fi
    done
fi
if [[ -z "$llvm_profdata" || -z "$llvm_cov" ]]; then
    echo 'Нужны llvm-profdata и llvm-cov.' >&2
    exit 1
fi

export RUSTFLAGS='-C instrument-coverage'
export CARGO_TARGET_DIR="$PWD/target/coverage"
export LLVM_PROFILE_FILE="$CARGO_TARGET_DIR/%p-%m.profraw"
mkdir -p "$CARGO_TARGET_DIR"
find "$CARGO_TARGET_DIR" -maxdepth 1 -name '*.profraw' -delete
cargo test --offline

shopt -s nullglob
profiles=("$CARGO_TARGET_DIR"/*.profraw)
if (( ${#profiles[@]} == 0 )); then
    echo 'Тесты не создали данные покрытия.' >&2
    exit 1
fi
"$llvm_profdata" merge -sparse "${profiles[@]}" -o "$CARGO_TARGET_DIR/merged.profdata"

objects=()
while IFS= read -r -d '' executable; do
    objects+=(-object "$executable")
done < <(find "$CARGO_TARGET_DIR/debug/deps" -maxdepth 1 -type f -executable \
    \( -name 'lab3-*' -o -name 'serialization-*' -o -name 'structures-*' \) -print0)
if (( ${#objects[@]} == 0 )); then
    echo 'Не найдены исполняемые файлы тестов.' >&2
    exit 1
fi

ignore='/.cargo/|/rustc/|/target/|tests/'
"$llvm_cov" report "${objects[@]}" -instr-profile="$CARGO_TARGET_DIR/merged.profdata" \
    -ignore-filename-regex="$ignore" | tee "$CARGO_TARGET_DIR/summary.txt"
# без -output-dir llvm-cov выводит все файлы в один HTML-документ
rm -rf coverage
mkdir -p coverage
"$llvm_cov" show "${objects[@]}" -instr-profile="$CARGO_TARGET_DIR/merged.profdata" \
    -ignore-filename-regex="$ignore" -format=html -show-line-counts-or-regions > coverage/index.html
echo "HTML: $PWD/coverage/index.html"
