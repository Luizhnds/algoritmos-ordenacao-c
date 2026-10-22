#include "sorting.h"

#include <stdlib.h>
#include <string.h>

static bool less_than(int left, int right, SortStats *stats) {
    ++stats->comparisons;
    return left < right;
}

static void swap_values(int *left, int *right, SortStats *stats) {
    if (left == right) {
        return;
    }
    int temp = *left;
    *left = *right;
    *right = temp;
    stats->writes += 2;
}

static void insertion_sort(int *values, size_t count, SortStats *stats) {
    for (size_t i = 1; i < count; ++i) {
        int item = values[i];
        size_t j = i;
        while (j > 0 && less_than(item, values[j - 1], stats)) {
            values[j] = values[j - 1];
            ++stats->writes;
            --j;
        }
        values[j] = item;
        ++stats->writes;
    }
}

/* O merge usa memoria auxiliar O(n), mas as escritas medidas sao so as do vetor principal. */
static void merge_ranges(int *values, int *buffer, size_t begin, size_t middle,
                         size_t end, SortStats *stats) {
    size_t left = begin;
    size_t right = middle;
    size_t out = begin;

    while (left < middle && right < end) {
        /* Em empate, preserva a ordem da metade esquerda (algoritmo estavel). */
        if (!less_than(values[right], values[left], stats)) {
            buffer[out++] = values[left++];
        } else {
            buffer[out++] = values[right++];
        }
    }
    while (left < middle) {
        buffer[out++] = values[left++];
    }
    while (right < end) {
        buffer[out++] = values[right++];
    }
    for (size_t i = begin; i < end; ++i) {
        values[i] = buffer[i];
        ++stats->writes;
    }
}

static void merge_sort_range(int *values, int *buffer, size_t begin,
                             size_t end, SortStats *stats) {
    if (end - begin < 2) {
        return;
    }
    size_t middle = begin + (end - begin) / 2;
    merge_sort_range(values, buffer, begin, middle, stats);
    merge_sort_range(values, buffer, middle, end, stats);
    merge_ranges(values, buffer, begin, middle, end, stats);
}

/* Mediana de tres evita o pivo extremo comum em vetores ja ordenados. */
static int median_of_three(int a, int b, int c, SortStats *stats) {
    if (less_than(a, b, stats)) {
        if (less_than(b, c, stats)) return b;
        return less_than(a, c, stats) ? c : a;
    }
    if (less_than(a, c, stats)) return a;
    return less_than(b, c, stats) ? c : b;
}

/* Particao em tres faixas: menores, iguais e maiores que o pivo.
 * Recorre primeiro na parte menor para limitar a profundidade da pilha. */
static void quick_sort_range(int *values, size_t begin, size_t end,
                             SortStats *stats) {
    while (end - begin > 1) {
        size_t middle = begin + (end - begin) / 2;
        int pivot = median_of_three(values[begin], values[middle],
                                    values[end - 1], stats);
        size_t lower = begin;
        size_t current = begin;
        size_t upper = end;

        while (current < upper) {
            if (less_than(values[current], pivot, stats)) {
                swap_values(&values[lower], &values[current], stats);
                ++lower;
                ++current;
            } else if (less_than(pivot, values[current], stats)) {
                --upper;
                swap_values(&values[current], &values[upper], stats);
            } else {
                ++current;
            }
        }

        if (lower - begin < end - upper) {
            quick_sort_range(values, begin, lower, stats);
            begin = upper;
        } else {
            quick_sort_range(values, upper, end, stats);
            end = lower;
        }
    }
}

static void sift_down(int *values, size_t root, size_t count, SortStats *stats) {
    /* root < count; esta formula evita overflow em 2 * root + 1. */
    while (root < count / 2 + count % 2) {
        size_t child = 2 * root + 1;
        if (child >= count) break;

        if (child + 1 < count && less_than(values[child], values[child + 1], stats)) {
            ++child;
        }
        if (!less_than(values[root], values[child], stats)) break;
        swap_values(&values[root], &values[child], stats);
        root = child;
    }
}

static void heap_sort(int *values, size_t count, SortStats *stats) {
    for (size_t i = count / 2; i > 0; --i) {
        sift_down(values, i - 1, count, stats);
    }
    for (size_t end = count; end > 1; --end) {
        swap_values(&values[0], &values[end - 1], stats);
        sift_down(values, 0, end - 1, stats);
    }
}

bool sort_ints(int *values, size_t count, SortAlgorithm algorithm, SortStats *stats) {
    if (stats == NULL || (count > 0 && values == NULL) ||
        (unsigned)algorithm >= (unsigned)SORT_ALGORITHM_COUNT) {
        return false;
    }
    *stats = (SortStats){0, 0};
    if (count < 2) return true;

    switch (algorithm) {
        case SORT_QUICK:
            quick_sort_range(values, 0, count, stats);
            return true;
        case SORT_MERGE: {
            if (count > SIZE_MAX / sizeof(*values)) return false;
            int *buffer = malloc(count * sizeof(*buffer));
            if (buffer == NULL) return false;
            merge_sort_range(values, buffer, 0, count, stats);
            free(buffer);
            return true;
        }
        case SORT_HEAP:
            heap_sort(values, count, stats);
            return true;
        case SORT_INSERTION:
            insertion_sort(values, count, stats);
            return true;
        default:
            return false;
    }
}

const char *sort_algorithm_name(SortAlgorithm algorithm) {
    switch (algorithm) {
        case SORT_QUICK: return "quick";
        case SORT_MERGE: return "merge";
        case SORT_HEAP: return "heap";
        case SORT_INSERTION: return "insertion";
        default: return "desconhecido";
    }
}

bool sort_parse_algorithm(const char *name, SortAlgorithm *algorithm) {
    if (name == NULL || algorithm == NULL) return false;
    for (int a = 0; a < SORT_ALGORITHM_COUNT; ++a) {
        if (strcmp(name, sort_algorithm_name((SortAlgorithm)a)) == 0) {
            *algorithm = (SortAlgorithm)a;
            return true;
        }
    }
    return false;
}
