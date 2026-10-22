#define _POSIX_C_SOURCE 200809L
#include "sorting.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ELEMENTS 1000000U

static bool read_integer(int *value) {
    char token[64];
    if (scanf("%63s", token) != 1) return false;
    errno = 0;
    char *end = NULL;
    long parsed = strtol(token, &end, 10);
    if (errno == ERANGE || end == token || *end != '\0' ||
        parsed < INT_MIN || parsed > INT_MAX) return false;
    *value = (int)parsed;
    return true;
}

static bool read_count(size_t *count) {
    char token[64];
    if (scanf("%63s", token) != 1 || token[0] == '-') return false;
    errno = 0;
    char *end = NULL;
    unsigned long long parsed = strtoull(token, &end, 10);
    if (errno == ERANGE || end == token || *end != '\0' ||
        parsed > MAX_ELEMENTS) return false;
    *count = (size_t)parsed;
    return true;
}

static double elapsed_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 +
           (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main(int argc, char **argv) {
    SortAlgorithm algorithm;
    if (argc != 2 || !sort_parse_algorithm(argv[1], &algorithm)) {
        fprintf(stderr, "Uso: %s <quick|merge|heap|insertion>\n", argv[0]);
        fprintf(stderr, "Entrada: N seguido de N numeros inteiros.\n");
        return EXIT_FAILURE;
    }

    size_t count;
    if (!read_count(&count)) {
        fprintf(stderr, "Erro: tamanho invalido (aceito de 0 a %u).\n", MAX_ELEMENTS);
        return EXIT_FAILURE;
    }
    int *values = NULL;
    if (count > 0) {
        values = malloc(count * sizeof(*values));
        if (values == NULL) {
            fprintf(stderr, "Erro: memoria insuficiente.\n");
            return EXIT_FAILURE;
        }
    }

    for (size_t i = 0; i < count; ++i) {
        if (!read_integer(&values[i])) {
            fprintf(stderr, "Erro: elemento %zu invalido ou ausente.\n", i + 1);
            free(values);
            return EXIT_FAILURE;
        }
    }

    SortStats stats;
    struct timespec start, finish;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0 ||
        !sort_ints(values, count, algorithm, &stats) ||
        clock_gettime(CLOCK_MONOTONIC, &finish) != 0) {
        fprintf(stderr, "Erro: nao foi possivel executar ou medir a ordenacao.\n");
        free(values);
        return EXIT_FAILURE;
    }

    printf("Vetor ordenado:");
    for (size_t i = 0; i < count; ++i) {
        printf(" %d", values[i]);
    }
    printf("\nTempo: %.6f ms\n", elapsed_ms(start, finish));
    printf("Comparacoes: %" PRIu64 "\n", stats.comparisons);
    printf("Escritas no vetor: %" PRIu64 "\n", stats.writes);
    free(values);
    return EXIT_SUCCESS;
}
