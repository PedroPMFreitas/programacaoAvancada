#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
=============================================================
Trabalho 11 — Gráficos Comparativos de Evasão de Colisão
=============================================================
Para usar no Google Colab:
1. Execute o benchmark no programa C++ (tecla F1)
2. O arquivo resultados_simulacao.csv será gerado na pasta build/
3. Faça upload desse CSV no Colab
4. Cole este script em uma célula e execute

Ou copie cada seção (separada por #%%) em células diferentes.
"""

#%% ============================================================
# CÉLULA 1: Upload do CSV e carregamento dos dados
# ==============================================================
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from google.colab import files

# Upload do arquivo CSV
print("Faça upload do arquivo resultados_simulacao.csv")
uploaded = files.upload()

# Carrega o CSV
csv_filename = list(uploaded.keys())[0]
df = pd.read_csv(csv_filename)

print(f"\n✅ Dados carregados: {len(df)} registros")
print(f"Métodos encontrados: {list(df['Metodo_Utilizado'].unique())}")
print(f"Quantidades de agentes: {sorted(df['Quantidade_Agentes'].unique())}")
print("\nPreview dos dados:")
display(df)

#%% ============================================================
# CÉLULA 2: Configuração de cores e labels
# ==============================================================
CORES = {
    "Direta": "#e74c3c",
    "Indireta": "#3498db",
    "Sem_Comunicacao": "#2ecc71"
}
MARCADORES = {
    "Direta": "o",
    "Indireta": "s",
    "Sem_Comunicacao": "^"
}
LABELS = {
    "Direta": "Comunicação Direta (Mediator + RVO2)",
    "Indireta": "Comunicação Indireta (Blackboard)",
    "Sem_Comunicacao": "Sem Comunicação (Reativo Local)"
}

plt.rcParams.update({
    'figure.figsize': (10, 6),
    'font.size': 12,
    'axes.grid': True,
    'grid.alpha': 0.3,
    'grid.linestyle': '--'
})

print("✅ Configurações de estilo carregadas")

#%% ============================================================
# CÉLULA 3: GRÁFICO A — Escalabilidade (Desempenho Computacional)
# Responde: "O que acontece quando colocamos mais agentes?"
# ==============================================================
fig, ax = plt.subplots(figsize=(10, 6))

for metodo in df['Metodo_Utilizado'].unique():
    subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
    ax.plot(
        subset['Quantidade_Agentes'],
        subset['Tempo_Computacional_Medio_ms'],
        marker=MARCADORES.get(metodo, 'o'),
        color=CORES.get(metodo, '#333'),
        linewidth=2.5,
        markersize=8,
        label=LABELS.get(metodo, metodo)
    )

ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
ax.set_ylabel('Tempo Computacional Médio por Frame (ms)', fontsize=13, fontweight='bold')
ax.set_title('A) Gráfico de Escalabilidade — Desempenho Computacional',
             fontsize=15, fontweight='bold', pad=15)
ax.legend(fontsize=11, loc='upper left')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig('grafico_escalabilidade.png', dpi=150, bbox_inches='tight')
plt.show()
print("✅ Gráfico A salvo: grafico_escalabilidade.png")

#%% ============================================================
# CÉLULA 4: GRÁFICO B — Qualidade da Rota
# Responde: "Existem problemas de conflito e qualidade de trajeto?"
# ==============================================================
fig, ax = plt.subplots(figsize=(10, 6))

metodos = list(df['Metodo_Utilizado'].unique())
agent_counts = sorted(df['Quantidade_Agentes'].unique())
n_methods = len(metodos)
bar_width = 0.25
x = np.arange(len(agent_counts))

for i, metodo in enumerate(metodos):
    subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
    valores = []
    for ac in agent_counts:
        row = subset[subset['Quantidade_Agentes'] == ac]
        valores.append(row['Distancia_Extra_Percorrida'].values[0] if len(row) > 0 else 0)

    offset = (i - n_methods / 2 + 0.5) * bar_width
    bars = ax.bar(
        x + offset, valores, bar_width,
        color=CORES.get(metodo, '#333'),
        label=LABELS.get(metodo, metodo),
        edgecolor='white', linewidth=0.8
    )
    for bar, val in zip(bars, valores):
        if val > 0:
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1,
                    f'{val:.0f}', ha='center', va='bottom', fontsize=8, fontweight='bold')

ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
ax.set_ylabel('Distância Extra Percorrida (px, média)', fontsize=13, fontweight='bold')
ax.set_title('B) Gráfico de Qualidade da Rota — Eficiência de Trajeto',
             fontsize=15, fontweight='bold', pad=15)
ax.set_xticks(x)
ax.set_xticklabels(agent_counts)
ax.legend(fontsize=10)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig('grafico_qualidade_rota.png', dpi=150, bbox_inches='tight')
plt.show()
print("✅ Gráfico B salvo: grafico_qualidade_rota.png")

#%% ============================================================
# CÉLULA 5: GRÁFICO C — Sucesso na Evasão (Total de Colisões)
# Responde: "O que queremos evitar?"
# ==============================================================
fig, ax = plt.subplots(figsize=(10, 6))

for i, metodo in enumerate(metodos):
    subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
    valores = []
    for ac in agent_counts:
        row = subset[subset['Quantidade_Agentes'] == ac]
        valores.append(int(row['Total_Colisoes'].values[0]) if len(row) > 0 else 0)

    offset = (i - n_methods / 2 + 0.5) * bar_width
    bars = ax.bar(
        x + offset, valores, bar_width,
        color=CORES.get(metodo, '#333'),
        label=LABELS.get(metodo, metodo),
        edgecolor='white', linewidth=0.8
    )
    for bar, val in zip(bars, valores):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.5,
                f'{val}', ha='center', va='bottom', fontsize=9, fontweight='bold')

ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
ax.set_ylabel('Total de Colisões Detectadas', fontsize=13, fontweight='bold')
ax.set_title('C) Gráfico de Sucesso — Evitando Colisões',
             fontsize=15, fontweight='bold', pad=15)
ax.set_xticks(x)
ax.set_xticklabels(agent_counts)
ax.legend(fontsize=10)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig('grafico_colisoes.png', dpi=150, bbox_inches='tight')
plt.show()
print("✅ Gráfico C salvo: grafico_colisoes.png")

#%% ============================================================
# CÉLULA 6: GRÁFICO D — Tempo Total de Conclusão
# Responde: "Quanto tempo leva para todos os agentes chegarem?"
# ==============================================================
fig, ax = plt.subplots(figsize=(10, 6))

for metodo in df['Metodo_Utilizado'].unique():
    subset = df[df['Metodo_Utilizado'] == metodo].sort_values('Quantidade_Agentes')
    ax.plot(
        subset['Quantidade_Agentes'],
        subset['Tempo_Total_Conclusao_s'],
        marker=MARCADORES.get(metodo, 'o'),
        color=CORES.get(metodo, '#333'),
        linewidth=2.5,
        markersize=8,
        label=LABELS.get(metodo, metodo)
    )

ax.set_xlabel('Quantidade de Agentes', fontsize=13, fontweight='bold')
ax.set_ylabel('Tempo Total de Conclusão (s)', fontsize=13, fontweight='bold')
ax.set_title('D) Tempo Total para Todos os Agentes Chegarem ao Destino',
             fontsize=15, fontweight='bold', pad=15)
ax.legend(fontsize=11, loc='upper left')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0)
plt.tight_layout()
plt.savefig('grafico_tempo_conclusao.png', dpi=150, bbox_inches='tight')
plt.show()
print("✅ Gráfico D salvo: grafico_tempo_conclusao.png")

#%% ============================================================
# CÉLULA 7: Tabela Resumo + Download dos gráficos
# ==============================================================
print("=" * 60)
print("  TABELA RESUMO DOS RESULTADOS")
print("=" * 60)
display(df.style.format({
    'Tempo_Computacional_Medio_ms': '{:.4f}',
    'Tempo_Total_Conclusao_s': '{:.2f}',
    'Distancia_Extra_Percorrida': '{:.1f}'
}).set_caption("Resultados da Simulação"))

# Download de todos os gráficos
print("\n📥 Fazendo download dos gráficos...")
for f in ['grafico_escalabilidade.png', 'grafico_qualidade_rota.png',
          'grafico_colisoes.png', 'grafico_tempo_conclusao.png']:
    try:
        files.download(f)
    except:
        print(f"  ⚠️ Não foi possível baixar {f} automaticamente")

print("\n✅ Concluído! Use os gráficos no seu relatório.")
