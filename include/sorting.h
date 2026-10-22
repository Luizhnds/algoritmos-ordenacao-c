#ifndef SORTING_H
#define SORTING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SORT_QUICK,
    SORT_MERGE,
    SORT_HEAP,
    SORT_INSERTION,
    SORT_ALGORITHM_COUNT
} SortAlgorithm;

/* Comparacoes entre valores do vetor; escritas no vetor principal. */
typedef struct {
    uint64_t comparisons;
    uint64_t writes;
} SortStats;

/* Retorna false apenas para parametros invalidos ou falha de memoria. */
bool sort_ints(int *values, size_t count, SortAlgorithm algorithm, SortStats *stats);
bool sort_parse_algorithm(const char *name, SortAlgorithm *algorithm);
const char *sort_algorithm_name(SortAlgorithm algorithm);

#endif
