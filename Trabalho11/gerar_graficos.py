#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Gerador de Gráficos Comparativos — Trabalho 11
Lê o arquivo resultados_simulacao.csv e gera 3 gráficos:
  A) Escalabilidade (Tempo Computacional vs Agentes)
  B) Qualidade de Rota (Distância Extra por Método)
  C) Sucesso na Evasão (Total de Colisões por Método)

Uso:
  python3 gerar_graficos.py
  python3 gerar_graficos.py resultados_simulacao.csv
"""

import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

# Usa backend não-interativo se não houver display
matplotlib.use('Agg')

def load_data(csv_path="resultados_simulacao.csv"):
    """Carrega dados do CSV gerado pelo SimulationLogger."""
    if not os.path.exists(csv_path):
        # Procura também no diretório build/
        alt_path = os.path.join("build", csv_path)
        if os.path.exists(alt_path):
            csv_path = alt_path
        else:
            print(f"ERRO: Arquivo '{csv_path}' não encontrado!")
            print("Execute o benchmark no programa (F1) primeiro.")
            sys.exit(1)

    df = pd.read_csv(csv_path)
    print(f"Dados carregados: {len(df)} registros de '{csv_path}'")
    print(f"Métodos: {df['Metodo_Utilizado'].unique()}")
    print(f"Agentes: {sorted(df['Quantidade_Agentes'].unique())}")
    print()
    return df


def grafico_escalabilidade(df, save_dir="."):
    """
    Gráfico A: Escalabilidade (Desempenho)
    Linha — Tempo Computacional Médio (ms) vs Quantidade de Agentes
    Responde: "O que acontece quando colocamos mais agentes?"
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    cores = {
        "Direta": "#e74c3c",       # Vermelho
        "Indireta": "#3498db",     # Azul
        "Sem_Comunicacao": "#2ecc71"  # Verde
    }
    marcadores = {
        "Direta": "o",
        "Indireta": "s",
        "Sem_Comunicacao": "^"
    }
    labels = {
        "Direta": "Comunicação Direta (Mediator + RVO2)",
        "Indireta": "Comunicação Indireta (Blackboard)",
        "Sem_Comunicacao": "Sem Comunicação (Reativo)"
    }

    for metodo in df['Metodo_Utilizado'].unique():
        subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
        ax.plot(
            subset['Quantidade_Agentes'],
            subset['Tempo_Computacional_Medio_ms'],
            marker=marcadores.get(metodo, 'o'),
            color=cores.get(metodo, '#333333'),
            linewidth=2.5,
            markersize=8,
            label=labels.get(metodo, metodo)
        )

    ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
    ax.set_ylabel('Tempo Computacional Médio por Frame (ms)', fontsize=13, fontweight='bold')
    ax.set_title('A) Gráfico de Escalabilidade — Desempenho Computacional',
                 fontsize=15, fontweight='bold', pad=15)
    ax.legend(fontsize=11, loc='upper left')
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.set_xlim(left=0)
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    path = os.path.join(save_dir, "grafico_escalabilidade.png")
    fig.savefig(path, dpi=150, bbox_inches='tight')
    print(f"Salvo: {path}")
    plt.close(fig)


def grafico_qualidade_rota(df, save_dir="."):
    """
    Gráfico B: Qualidade da Rota
    Barras Agrupadas — Distância Extra Percorrida (média) por método
    Responde: "Existem muitos problemas de conflito e qualidade de trajeto?"
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    metodos = df['Metodo_Utilizado'].unique()
    agent_counts = sorted(df['Quantidade_Agentes'].unique())
    n_methods = len(metodos)
    bar_width = 0.25
    x = np.arange(len(agent_counts))

    cores = {
        "Direta": "#e74c3c",
        "Indireta": "#3498db",
        "Sem_Comunicacao": "#2ecc71"
    }
    labels = {
        "Direta": "Com. Direta (RVO2)",
        "Indireta": "Com. Indireta (Blackboard)",
        "Sem_Comunicacao": "Sem Comunicação (Reativo)"
    }

    for i, metodo in enumerate(metodos):
        subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
        valores = []
        for ac in agent_counts:
            row = subset[subset['Quantidade_Agentes'] == ac]
            valores.append(row['Distancia_Extra_Percorrida'].values[0] if len(row) > 0 else 0)

        offset = (i - n_methods / 2 + 0.5) * bar_width
        bars = ax.bar(
            x + offset, valores, bar_width,
            color=cores.get(metodo, '#333333'),
            label=labels.get(metodo, metodo),
            edgecolor='white',
            linewidth=0.8
        )
        # Adiciona valores em cima das barras
        for bar, val in zip(bars, valores):
            if val > 0:
                ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 1,
                        f'{val:.0f}', ha='center', va='bottom', fontsize=8, fontweight='bold')

    ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
    ax.set_ylabel('Distância Extra Percorrida (px, média)', fontsize=13, fontweight='bold')
    ax.set_title('B) Gráfico de Qualidade da Rota — Eficiência de Trajeto',
                 fontsize=15, fontweight='bold', pad=15)
    ax.set_xticks(x)
    ax.set_xticklabels(agent_counts)
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3, linestyle='--', axis='y')
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    path = os.path.join(save_dir, "grafico_qualidade_rota.png")
    fig.savefig(path, dpi=150, bbox_inches='tight')
    print(f"Salvo: {path}")
    plt.close(fig)


def grafico_colisoes(df, save_dir="."):
    """
    Gráfico C: Sucesso na Evasão de Colisões
    Barras Agrupadas — Total de Colisões por método e quantidade de agentes
    Responde: "O que queremos evitar?"
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    metodos = df['Metodo_Utilizado'].unique()
    agent_counts = sorted(df['Quantidade_Agentes'].unique())
    n_methods = len(metodos)
    bar_width = 0.25
    x = np.arange(len(agent_counts))

    cores = {
        "Direta": "#e74c3c",
        "Indireta": "#3498db",
        "Sem_Comunicacao": "#2ecc71"
    }
    labels = {
        "Direta": "Com. Direta (RVO2)",
        "Indireta": "Com. Indireta (Blackboard)",
        "Sem_Comunicacao": "Sem Comunicação (Reativo)"
    }

    for i, metodo in enumerate(metodos):
        subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
        valores = []
        for ac in agent_counts:
            row = subset[subset['Quantidade_Agentes'] == ac]
            valores.append(row['Total_Colisoes'].values[0] if len(row) > 0 else 0)

        offset = (i - n_methods / 2 + 0.5) * bar_width
        bars = ax.bar(
            x + offset, valores, bar_width,
            color=cores.get(metodo, '#333333'),
            label=labels.get(metodo, metodo),
            edgecolor='white',
            linewidth=0.8
        )
        # Adiciona valores
        for bar, val in zip(bars, valores):
            ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.5,
                    f'{val}', ha='center', va='bottom', fontsize=9, fontweight='bold')

    ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
    ax.set_ylabel('Total de Colisões Detectadas', fontsize=13, fontweight='bold')
    ax.set_title('C) Gráfico de Sucesso — Evitando Colisões',
                 fontsize=15, fontweight='bold', pad=15)
    ax.set_xticks(x)
    ax.set_xticklabels(agent_counts)
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3, linestyle='--', axis='y')
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    path = os.path.join(save_dir, "grafico_colisoes.png")
    fig.savefig(path, dpi=150, bbox_inches='tight')
    print(f"Salvo: {path}")
    plt.close(fig)


def grafico_tempo_conclusao(df, save_dir="."):
    """
    Gráfico D (bônus): Tempo até Conclusão
    Linha — Tempo Total (s) para todos os agentes chegarem ao destino
    """
    fig, ax = plt.subplots(figsize=(10, 6))

    cores = {
        "Direta": "#e74c3c",
        "Indireta": "#3498db",
        "Sem_Comunicacao": "#2ecc71"
    }
    marcadores = {
        "Direta": "o",
        "Indireta": "s",
        "Sem_Comunicacao": "^"
    }
    labels = {
        "Direta": "Comunicação Direta (RVO2)",
        "Indireta": "Comunicação Indireta (Blackboard)",
        "Sem_Comunicacao": "Sem Comunicação (Reativo)"
    }

    for metodo in df['Metodo_Utilizado'].unique():
        subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
        ax.plot(
            subset['Quantidade_Agentes'],
            subset['Tempo_Total_Conclusao_s'],
            marker=marcadores.get(metodo, 'o'),
            color=cores.get(metodo, '#333333'),
            linewidth=2.5,
            markersize=8,
            label=labels.get(metodo, metodo)
        )

    ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
    ax.set_ylabel('Tempo Total de Conclusão (s)', fontsize=13, fontweight='bold')
    ax.set_title('D) Tempo Total para Todos os Agentes Chegarem ao Destino',
                 fontsize=15, fontweight='bold', pad=15)
    ax.legend(fontsize=11, loc='upper left')
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.set_xlim(left=0)
    ax.set_ylim(bottom=0)

    plt.tight_layout()
    path = os.path.join(save_dir, "grafico_tempo_conclusao.png")
    fig.savefig(path, dpi=150, bbox_inches='tight')
    print(f"Salvo: {path}")
    plt.close(fig)


def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "resultados_simulacao.csv"

    print("=" * 60)
    print("  GERADOR DE GRÁFICOS — Trabalho 11")
    print("  Comparativo dos 3 Métodos de Evasão de Colisão")
    print("=" * 60)
    print()

    df = load_data(csv_path)

    # Diretório para salvar gráficos
    save_dir = "graficos"
    os.makedirs(save_dir, exist_ok=True)

    print("Gerando gráficos...\n")

    grafico_escalabilidade(df, save_dir)
    grafico_qualidade_rota(df, save_dir)
    grafico_colisoes(df, save_dir)
    grafico_tempo_conclusao(df, save_dir)

    print(f"\nTodos os gráficos salvos em: {save_dir}/")
    print("Arquivos gerados:")
    print("  - grafico_escalabilidade.png    (A: Desempenho)")
    print("  - grafico_qualidade_rota.png    (B: Qualidade)")
    print("  - grafico_colisoes.png          (C: Sucesso)")
    print("  - grafico_tempo_conclusao.png   (D: Tempo)")
    print()

    # Mostra tabela resumo
    print("=" * 60)
    print("  TABELA RESUMO")
    print("=" * 60)
    print(df.to_string(index=False))
    print()


if __name__ == "__main__":
    main()
