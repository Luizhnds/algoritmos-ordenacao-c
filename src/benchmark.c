#define _POSIX_C_SOURCE 200809L
#include "sorting.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BENCHMARK_SIZE 50000U
#define MAX_REPETITIONS 100U

typedef enum {
    RANDOM_VALUES,
    SORTED_VALUES,
    REVERSE_VALUES,
    FEW_DISTINCT_VALUES,
    SCENARIO_COUNT
} Scenario;

static const char *scenario_name(Scenario scenario) {
    switch (scenario) {
        case RANDOM_VALUES: return "aleatorio";
        case SORTED_VALUES: return "ordenado";
        case REVERSE_VALUES: return "reverso";
        case FEW_DISTINCT_VALUES: return "repetidos";
        default: return "desconhecido";
    }
}

/* PRNG deterministico: execucoes com os mesmos parametros geram os mesmos dados. */
static uint32_t rng_state = UINT32_C(0xC0FFEE);
static uint32_t next_random(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

static int compare_ints(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static bool parse_number(const char *input, unsigned long long min,
                         unsigned long long max, size_t *output) {
    if (input == NULL || *input == '-' || *input == '\0') return false;
    errno = 0;
    char *end = NULL;
    unsigned long long result = strtoull(input, &end, 10);
    if (errno == ERANGE || end == input || *end != '\0' ||
        result < min || result > max) return false;
    *output = (size_t)result;
    return true;
}

static void generate_values(int *values, size_t count, Scenario scenario) {
    for (size_t i = 0; i < count; ++i) {
        values[i] = scenario == FEW_DISTINCT_VALUES
            ? (int)(next_random() % 7U)
            : (int)(next_random() % 200001U) - 100000;
    }
    if (scenario == SORTED_VALUES || scenario == REVERSE_VALUES) {
        qsort(values, count, sizeof(*values), compare_ints);
        if (scenario == REVERSE_VALUES) {
            for (size_t i = 0; i < count / 2; ++i) {
                int tmp = values[i];
                values[i] = values[count - 1 - i];
                values[count - 1 - i] = tmp;
            }
        }
    }
}

static double elapsed_ms(struct timespec start, struct timespec finish) {
    return (double)(finish.tv_sec - start.tv_sec) * 1000.0 +
           (double)(finish.tv_nsec - start.tv_nsec) / 1000000.0;
}

static bool benchmark(FILE *csv, size_t count, size_t repetitions) {
    int *original = malloc(count * sizeof(*original));
    int *expected = malloc(count * sizeof(*expected));
    int *working = malloc(count * sizeof(*working));
    if (original == NULL || expected == NULL || working == NULL) {
        fprintf(stderr, "Erro de alocacao para %zu elementos.\n", count);
        free(original);
        free(expected);
        free(working);
        return false;
    }

    for (int scenario = 0; scenario < SCENARIO_COUNT; ++scenario) {
        for (size_t repetition = 1; repetition <= repetitions; ++repetition) {
            generate_values(original, count, (Scenario)scenario);
            memcpy(expected, original, count * sizeof(*expected));
            qsort(expected, count, sizeof(*expected), compare_ints);

            /* Rotacionar a ordem reduz favorecimento por cache/ordem de execucao. */
            for (int order = 0; order < SORT_ALGORITHM_COUNT; ++order) {
                SortAlgorithm algorithm = (SortAlgorithm)((order + (int)(repetition - 1)) %
                                                          SORT_ALGORITHM_COUNT);
                memcpy(working, original, count * sizeof(*working));
                SortStats stats;
                struct timespec start, finish;
                if (clock_gettime(CLOCK_MONOTONIC, &start) != 0 ||
                    !sort_ints(working, count, algorithm, &stats) ||
                    clock_gettime(CLOCK_MONOTONIC, &finish) != 0) {
                    fprintf(stderr, "Falha ao medir %s.\n", sort_algorithm_name(algorithm));
                    free(original); free(expected); free(working);
                    return false;
                }
                if (memcmp(working, expected, count * sizeof(*working)) != 0) {
                    fprintf(stderr, "Falha de corretude em %s (n=%zu).\n",
                            sort_algorithm_name(algorithm), count);
                    free(original); free(expected); free(working);
                    return false;
                }
                if (fprintf(csv, "%s,%s,%zu,%zu,%.9f,%" PRIu64 ",%" PRIu64 "\n",
                            scenario_name((Scenario)scenario), sort_algorithm_name(algorithm),
                            count, repetition, elapsed_ms(start, finish),
                            stats.comparisons, stats.writes) < 0) {
                    fprintf(stderr, "Erro de escrita no CSV.\n");
                    free(original); free(expected); free(working);
                    return false;
                }
            }
        }
    }
    free(original);
    free(expected);
    free(working);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <arquivo.csv> <repeticoes> <tamanho1> [tamanho2 ...]\n", argv[0]);
        fprintf(stderr, "Limites: repeticoes de 1 a %u; tamanhos de 1 a %u.\n",
                MAX_REPETITIONS, MAX_BENCHMARK_SIZE);
        return EXIT_FAILURE;
    }

    size_t repetitions;
    if (!parse_number(argv[2], 1, MAX_REPETITIONS, &repetitions)) {
        fprintf(stderr, "Numero de repeticoes invalido.\n");
        return EXIT_FAILURE;
    }
    for (int i = 3; i < argc; ++i) {
        size_t count;
        if (!parse_number(argv[i], 1, MAX_BENCHMARK_SIZE, &count)) {
            fprintf(stderr, "Tamanho invalido: %s.\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    FILE *csv = fopen(argv[1], "w");
    if (csv == NULL) {
        perror("Nao foi possivel abrir o CSV (a pasta existe?)");
        return EXIT_FAILURE;
    }
    if (fprintf(csv, "cenario,algoritmo,tamanho,repeticao,tempo_ms,comparacoes,escritas\n") < 0) {
        fprintf(stderr, "Erro de escrita no CSV.\n");
        fclose(csv);
        return EXIT_FAILURE;
    }

    for (int i = 3; i < argc; ++i) {
        size_t count;
        (void)parse_number(argv[i], 1, MAX_BENCHMARK_SIZE, &count);
        if (!benchmark(csv, count, repetitions)) {
            fclose(csv);
            return EXIT_FAILURE;
        }
        printf("Concluido: %zu elementos, %zu repeticoes, 4 cenarios, 4 algoritmos.\n",
               count, repetitions);
    }
    if (fclose(csv) != 0) {
        perror("Erro ao fechar CSV");
        return EXIT_FAILURE;
    }
    printf("Resultados salvos em %s\n", argv[1]);
    return EXIT_SUCCESS;
}
