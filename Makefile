CC = gcc
CPPFLAGS = -Iinclude -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wconversion

.PHONY: all test bench graphs clean help
all: bin/ordenacao bin/benchmark

build/sorting.o: src/sorting.c include/sorting.h
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

bin/ordenacao: src/main.c build/sorting.o include/sorting.h
	@mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c build/sorting.o -o $@

bin/benchmark: src/benchmark.c build/sorting.o include/sorting.h
	@mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) src/benchmark.c build/sorting.o -o $@

bin/test_sorting: tests/test_sorting.c build/sorting.o include/sorting.h
	@mkdir -p bin
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_sorting.c build/sorting.o -o $@

test: bin/test_sorting bin/ordenacao
	./bin/test_sorting
	@bash tests/test_cli.sh ./bin/ordenacao

bench: bin/benchmark
	@mkdir -p resultados
	./bin/benchmark resultados/benchmarks.csv 5 100 1000 5000

graphs:
	python3 scripts/plot_results.py resultados/benchmarks.csv resultados/graficos

clean:
	rm -rf build bin resultados

help:
	@echo 'make          - compila os programas'
	@echo 'make test     - executa testes automatizados'
	@echo 'make bench    - executa o experimento padrao e cria CSV'
	@echo 'make graphs   - cria graficos com matplotlib (opcional)'
	@echo 'make clean    - remove arquivos gerados'
