"""Gera um grafico por cenario a partir do CSV produzido em C.

Uso: python3 scripts/plot_results.py <arquivo.csv> <pasta_saida>
"""
import csv
from collections import defaultdict
from pathlib import Path
from statistics import mean
import sys

try:
    import matplotlib.pyplot as plt
except ImportError:
    raise SystemExit("Instale matplotlib: python3 -m pip install -r requirements-graficos.txt")

TITULOS = {
    "aleatorio": "Dados aleatórios",
    "ordenado": "Dados ordenados",
    "reverso": "Dados em ordem inversa",
    "repetidos": "Dados com poucos valores distintos",
}
ALGORITMOS = ("quick", "merge", "heap", "insertion")


def main() -> None:
    if len(sys.argv) != 3:
        raise SystemExit("Uso: python3 scripts/plot_results.py <arquivo.csv> <pasta_saida>")
    source, destination = Path(sys.argv[1]), Path(sys.argv[2])
    groups = defaultdict(list)
    with source.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        required = {"cenario", "algoritmo", "tamanho", "tempo_ms"}
        if not required.issubset(set(reader.fieldnames or [])):
            raise SystemExit("CSV invalido: colunas obrigatorias ausentes")
        for row in reader:
            groups[(row["cenario"], row["algoritmo"], int(row["tamanho"]))].append(
                float(row["tempo_ms"])
            )

    if not groups:
        raise SystemExit("CSV sem dados para plotar")
    destination.mkdir(parents=True, exist_ok=True)
    scenarios = sorted({key[0] for key in groups})
    for scenario in scenarios:
        fig, ax = plt.subplots(figsize=(8, 5))
        for algorithm in ALGORITMOS:
            points = sorted((size, mean(values)) for (sc, algo, size), values in groups.items()
                            if sc == scenario and algo == algorithm)
            if points:
                ax.plot([x for x, _ in points], [y for _, y in points],
                        marker="o", linewidth=2, label=algorithm.capitalize())
        ax.set_title(f"Tempo médio de ordenação — {TITULOS.get(scenario, scenario)}")
        ax.set_xlabel("Quantidade de elementos")
        ax.set_ylabel("Tempo médio (ms)")
        ax.grid(alpha=0.25)
        ax.legend()
        fig.tight_layout()
        path = destination / f"{scenario}.png"
        fig.savefig(path, dpi=150)
        plt.close(fig)
        print(f"Gráfico salvo: {path}")


if __name__ == "__main__":
    main()
