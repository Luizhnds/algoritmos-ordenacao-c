#include "sorting.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void assert_sorted_matches_qsort(const int *original, size_t count) {
    int *expected = malloc((count ? count : 1) * sizeof(*expected));
    int *actual = malloc((count ? count : 1) * sizeof(*actual));
    assert(expected != NULL && actual != NULL);
    if (count > 0) memcpy(expected, original, count * sizeof(*expected));
    qsort(expected, count, sizeof(*expected), compare_ints);
    for (int i = 0; i < SORT_ALGORITHM_COUNT; ++i) {
        if (count > 0) memcpy(actual, original, count * sizeof(*actual));
        SortStats stats = {123, 456};
        assert(sort_ints(actual, count, (SortAlgorithm)i, &stats));
        assert(memcmp(actual, expected, count * sizeof(*expected)) == 0);
        if (count > 1) assert(stats.comparisons > 0);
    }
    free(expected);
    free(actual);
}

static unsigned rng_state = 0xC0FFEEU;
static unsigned next_random(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

int main(void) {
    int singleton[] = {42};
    int already[] = {-5, -4, 0, 1, 7, 100};
    int reverse[] = {100, 7, 1, 0, -4, -5};
    int repeated[] = {2, 2, 2, 2, 2, 2, 2};
    int mixed[] = {INT_MAX, INT_MIN, -1, 0, INT_MIN, INT_MAX, 7};
    assert_sorted_matches_qsort(NULL, 0);
    assert_sorted_matches_qsort(singleton, 1);
    assert_sorted_matches_qsort(already, sizeof(already) / sizeof(already[0]));
    assert_sorted_matches_qsort(reverse, sizeof(reverse) / sizeof(reverse[0]));
    assert_sorted_matches_qsort(repeated, sizeof(repeated) / sizeof(repeated[0]));
    assert_sorted_matches_qsort(mixed, sizeof(mixed) / sizeof(mixed[0]));

    int sample[2000];
    for (size_t size = 2; size <= 2000; size = size < 50 ? size + 1 : size * 2) {
        for (int trial = 0; trial < 12; ++trial) {
            for (size_t j = 0; j < size; ++j) {
                sample[j] = (int)(next_random() % 201U) - 100;
            }
            assert_sorted_matches_qsort(sample, size);
        }
    }

    SortStats stats = {1, 1};
    assert(sort_ints(NULL, 0, SORT_QUICK, &stats));
    assert(stats.comparisons == 0 && stats.writes == 0);
    assert(!sort_ints(NULL, 2, SORT_QUICK, &stats));
    assert(!sort_ints(singleton, 1, SORT_QUICK, NULL));
    assert(!sort_ints(singleton, 1, (SortAlgorithm)999, &stats));
    SortAlgorithm algorithm;
    assert(sort_parse_algorithm("merge", &algorithm) && algorithm == SORT_MERGE);
    assert(!sort_parse_algorithm("outro", &algorithm));
    assert(!sort_parse_algorithm(NULL, &algorithm));

    puts("Todos os testes passaram: casos-limite, algoritmos e vetores aleatorios.");
    return 0;
}
