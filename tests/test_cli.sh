#!/usr/bin/env bash
set -euo pipefail

exe="${1:-./bin/ordenacao}"

expect_success() {
    local alg="$1" input="$2" expected="$3" actual
    actual="$(printf '%s' "$input" | "$exe" "$alg")"
    if ! grep -Fq "$expected" <<< "$actual"; then
        printf 'Saida inesperada para %s\n%s\n' "$alg" "$actual" >&2
        exit 1
    fi
}

expect_failure() {
    local alg="$1" input="$2"
    if printf '%s' "$input" | "$exe" "$alg" >/dev/null 2>&1; then
        printf 'Entrada invalida foi aceita por %s: %s\n' "$alg" "$input" >&2
        exit 1
    fi
}

for alg in quick merge heap insertion; do
    expect_success "$alg" $'6\n3 -5 3 0 100 -5\n' 'Vetor ordenado: -5 -5 0 3 3 100'
    expect_success "$alg" $'0\n' 'Vetor ordenado:'
    expect_success "$alg" $'1\n-42\n' 'Vetor ordenado: -42'
done
expect_failure quick $'-1\n'
expect_failure merge $'1000001\n'
expect_failure heap $'2\n3 nao-inteiro\n'
expect_failure insertion $'2\n3\n'
expect_failure quick $'1\n2147483648\n'
expect_failure desconhecido $'1\n3\n'

echo 'Testes da interface e entradas invalidas: OK'
