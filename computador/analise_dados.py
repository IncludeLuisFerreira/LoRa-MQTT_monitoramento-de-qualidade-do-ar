#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
================================================================================
Nível 5 - Processamento Analítico Básico (TpM)
================================================================================
Script de análise analítica independente para consumir dados brutos gerados
pelo Nível 4 (mqtt_client.py) e extrair insights focados via janela temporal.

Arquitetura:
    - Entrada: arquivos JSON no diretório `dados/` (formato: [{date, luminosidade}])
    - Processamento: filtro temporal + cálculo estatístico (média)
    - Saída: gráfico de linha temporal com indicador de média (PNG + exibição)

Uso:
    python computador/analise_dados.py
    python computador/analise_dados.py --janela 30
    python computador/analise_dados.py --inicio "2026-05-23 14:30:00" --duracao 60
================================================================================
"""

import argparse
import json
import os
import sys
from datetime import datetime, timedelta
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# -----------------------------------------------------------------------------
# CONFIGURAÇÕES
# -----------------------------------------------------------------------------
DADOS_DIR = Path(__file__).resolve().parent / "dados"
FORMATO_DATA = "%Y-%m-%d %H:%M:%S"
DURACAO_PADRAO_JANELA_SEGUNDOS = 60  # 1 minuto (pode ser reduzido para 30s)

# -----------------------------------------------------------------------------
# CORE: LEITURA E PARSING
# -----------------------------------------------------------------------------

def carregar_todos_json(diretorio: Path) -> list[dict]:
    """
    Lê todos os arquivos *.json do diretório especificado, faz o parsing
    e retorna uma lista plana de registros.

    Suporta tanto JSON array quanto JSON lines (um objeto por linha).
    """
    if not diretorio.exists():
        raise FileNotFoundError(
            f"Diretório de dados não encontrado: {diretorio}\n"
            "Execute primeiro o mqtt_client.py para gerar dados."
        )

    arquivos_json = sorted(diretorio.glob("*.json"))
    if not arquivos_json:
        raise FileNotFoundError(
            f"Nenhum arquivo .json encontrado em: {diretorio}"
        )

    registros = []
    for arquivo in arquivos_json:
        try:
            with open(arquivo, "r", encoding="utf-8") as f:
                conteudo = f.read().strip()
                if not conteudo:
                    continue
                # Tenta interpretar como JSON array
                if conteudo.startswith("["):
                    dados = json.loads(conteudo)
                else:
                    # JSON lines: um objeto por linha
                    dados = [json.loads(linha) for linha in conteudo.splitlines() if linha.strip()]

                if isinstance(dados, list):
                    registros.extend(dados)
                else:
                    registros.append(dados)
        except (json.JSONDecodeError, UnicodeDecodeError, IOError) as e:
            print(f"⚠️  Ignorando {arquivo.name}: {e}")
            continue

    return registros


def parse_registros(registros: list[dict]) -> tuple[list[datetime], list[int]]:
    """
    Extrai e converte os campos 'date' e 'luminosidade' dos registros brutos.

    Returns:
        tupla (lista de datetime, lista de int) já ordenada cronologicamente.
    """
    parsed = []
    for i, item in enumerate(registros):
        try:
            data_str = item.get("date") or item.get("data") or item.get("timestamp")
            lum_val = item.get("luminosidade") or item.get("valor") or item.get("ldr")

            if data_str is None or lum_val is None:
                continue

            dt = datetime.strptime(str(data_str).strip(), FORMATO_DATA)
            lum = int(lum_val)
            parsed.append((dt, lum))
        except (ValueError, TypeError, KeyError) as e:
            print(f"⚠️  Registro {i} inválido ({e}): {item}")
            continue

    # Ordenação cronológica
    parsed.sort(key=lambda x: x[0])

    if not parsed:
        raise ValueError("Nenhum registro válido encontrado após parsing.")

    tempos = [p[0] for p in parsed]
    luminosidades = [p[1] for p in parsed]
    return tempos, luminosidades


# -----------------------------------------------------------------------------
# CORE: FILTRO TEMPORAL
# -----------------------------------------------------------------------------

def filtrar_janela(
    tempos: list[datetime],
    luminosidades: list[int],
    inicio: datetime | None = None,
    duracao_segundos: int = DURACAO_PADRAO_JANELA_SEGUNDOS,
) -> tuple[list[datetime], list[int], datetime, datetime]:
    """
    Isola na memória apenas os dados pertencentes à janela temporal especificada.

    Se `inicio` for None, utiliza a janela mais recente disponível nos dados
    (fim = último timestamp, início = fim - duracao).

    Returns:
        (tempos_filtrados, luminosidades_filtradas, inicio_efetivo, fim_efetivo)
    """
    if not tempos:
        raise ValueError("Lista de tempos vazia.")

    if inicio is None:
        # Janela mais recente: ancora no último dado disponível
        fim = tempos[-1]
        inicio = fim - timedelta(seconds=duracao_segundos)
    else:
        fim = inicio + timedelta(seconds=duracao_segundos)

    tempos_filt = []
    lum_filt = []

    for t, l in zip(tempos, luminosidades):
        if inicio <= t <= fim:
            tempos_filt.append(t)
            lum_filt.append(l)

    if not tempos_filt:
        raise ValueError(
            f"Nenhum dado encontrado na janela {inicio} → {fim} "
            f"(dados disponíveis: {tempos[0]} → {tempos[-1]})"
        )

    return tempos_filt, lum_filt, inicio, fim


# -----------------------------------------------------------------------------
# CORE: PROCESSAMENTO ANALÍTICO
# -----------------------------------------------------------------------------

def calcular_media(luminosidades: list[int]) -> float:
    """Calcula a média aritmética da lista de luminosidades."""
    if not luminosidades:
        return 0.0
    return sum(luminosidades) / len(luminosidades)


# -----------------------------------------------------------------------------
# CORE: VISUALIZAÇÃO
# -----------------------------------------------------------------------------

def plotar_analise(
    tempos: list[datetime],
    luminosidades: list[int],
    media: float,
    inicio: datetime,
    fim: datetime,
    salvar_png: bool = True,
) -> None:
    """
    Plota o gráfico de linha temporal da janela analisada com a média destacada.

    Critérios atendidos:
      - Eixo X em escala temporal real (datetime), não índices numéricos.
      - Linha tracejada vermelha indicando a média da janela.
      - Formatação legível de datas no eixo X.
    """
    fig, ax = plt.subplots(figsize=(12, 6))

    # Linha principal da série temporal
    ax.plot(
        tempos,
        luminosidades,
        marker="o",
        linestyle="-",
        color="#2E86AB",
        linewidth=2,
        markersize=6,
        label="Luminosidade (LDR)",
        zorder=3,
    )

    # Linha horizontal tracejada vermelha = média da JANELA
    ax.axhline(
        y=media,
        color="#E94F37",
        linestyle="--",
        linewidth=2,
        label=f"Média Janela = {media:.2f}",
        zorder=2,
    )

    # Preenchimento sutil entre a série e a média
    ax.fill_between(
        tempos,
        luminosidades,
        media,
        where=[l >= media for l in luminosidades],
        color="#2E86AB",
        alpha=0.15,
        interpolate=True,
    )
    ax.fill_between(
        tempos,
        luminosidades,
        media,
        where=[l < media for l in luminosidades],
        color="#E94F37",
        alpha=0.15,
        interpolate=True,
    )

    # Formatação do eixo X (tempo real)
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
    ax.xaxis.set_major_locator(mdates.SecondLocator(interval=max(5, (fim - inicio).seconds // 6)))

    # Rótulos e título
    duracao = (fim - inicio).total_seconds()
    ax.set_title(
        f"Análise Analítica Inicial — Nível 5 TpM\n"
        f"Janela: {inicio.strftime(FORMATO_DATA)} → {fim.strftime(FORMATO_DATA)} "
        f"({duracao:.0f}s)",
        fontsize=13,
        fontweight="bold",
        color="#1B1B1E",
    )
    ax.set_xlabel("Tempo (HH:MM:SS)", fontsize=11)
    ax.set_ylabel("Luminosidade (unidades ADC)", fontsize=11)

    # Grid e legenda
    ax.grid(True, linestyle=":", alpha=0.6, zorder=1)
    ax.legend(loc="best", framealpha=0.9)

    # Ajuste automático da inclinação das datas
    fig.autofmt_xdate(rotation=30)

    # Anotação da média no último ponto
    if tempos:
        ax.annotate(
            f"{media:.1f}",
            xy=(tempos[-1], media),
            xytext=(10, 5),
            textcoords="offset points",
            fontsize=10,
            color="#E94F37",
            fontweight="bold",
        )

    plt.tight_layout()

    # Salvamento opcional
    if salvar_png:
        timestamp_arquivo = datetime.now().strftime("%Y%m%d_%H%M%S")
        nome_png = f"analise_nivel5_{timestamp_arquivo}.png"
        caminho_png = Path(__file__).resolve().parent / "dados" / nome_png
        plt.savefig(caminho_png, dpi=150, bbox_inches="tight")
        print(f"📊 Gráfico salvo em: {caminho_png}")

    plt.show()


# -----------------------------------------------------------------------------
# ORQUESTRAÇÃO PRINCIPAL
# -----------------------------------------------------------------------------

def executar_analise(inicio_str: str | None, duracao: int, salvar: bool) -> None:
    """Pipeline completo: carga → parse → filtro → métricas → visualização."""
    print("=" * 60)
    print("🔬 NÍVEL 5 — PROCESSAMENTO ANALÍTICO BÁSICO (TpM)")
    print("=" * 60)

    # 1. Carga
    print(f"📂 Lendo arquivos JSON de: {DADOS_DIR}")
    registros_brutos = carregar_todos_json(DADOS_DIR)
    print(f"   └─ {len(registros_brutos)} registros brutos carregados")

    # 2. Parsing
    tempos, luminosidades = parse_registros(registros_brutos)
    print(f"📅 Período total disponível: {tempos[0]} → {tempos[-1]}")
    print(f"📊 Total de amostras válidas: {len(tempos)}")

    # 3. Filtro temporal
    inicio_dt = None
    if inicio_str:
        inicio_dt = datetime.strptime(inicio_str, FORMATO_DATA)
        print(f"⏱️  Janela solicitada: {inicio_str} (+{duracao}s)")
    else:
        print(f"⏱️  Janela automática (últimos {duracao}s)")

    tempos_janela, lum_janela, inicio_ef, fim_ef = filtrar_janela(
        tempos, luminosidades, inicio_dt, duracao
    )
    print(f"   └─ {len(tempos_janela)} amostras na janela ({inicio_ef} → {fim_ef})")

    # 4. Métricas
    media = calcular_media(lum_janela)
    print(f"📈 Média de luminosidade na janela: {media:.2f}")
    print(f"   └─ Mín: {min(lum_janela)} | Máx: {max(lum_janela)} | Amostras: {len(lum_janela)}")

    # 5. Visualização
    print("🎨 Gerando gráfico...")
    plotar_analise(tempos_janela, lum_janela, media, inicio_ef, fim_ef, salvar)
    print("✅ Análise concluída.")


# -----------------------------------------------------------------------------
# CLI
# -----------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Nível 5 TpM: Processamento Analítico Básico de dados IoT"
    )
    parser.add_argument(
        "--inicio",
        type=str,
        default=None,
        help=f"Data/hora de início da janela (formato: {FORMATO_DATA}). "
             "Se omitido, usa a janela mais recente.",
    )
    parser.add_argument(
        "--duracao",
        "--janela",
        type=int,
        default=DURACAO_PADRAO_JANELA_SEGUNDOS,
        help=f"Duração da janela de análise em segundos (padrão: {DURACAO_PADRAO_JANELA_SEGUNDOS}).",
    )
    parser.add_argument(
        "--no-save",
        action="store_true",
        help="Não salvar o gráfico como PNG (apenas exibir).",
    )
    parser.add_argument(
        "--dados-dir",
        type=str,
        default=None,
        help="Caminho alternativo para o diretório de dados JSON.",
    )

    args = parser.parse_args()

    global DADOS_DIR
    if args.dados_dir:
        DADOS_DIR = Path(args.dados_dir)

    try:
        executar_analise(
            inicio_str=args.inicio,
            duracao=args.duracao,
            salvar=not args.no_save,
        )
    except FileNotFoundError as e:
        print(f"❌ ERRO: {e}")
        sys.exit(1)
    except ValueError as e:
        print(f"❌ ERRO de processamento: {e}")
        sys.exit(2)
    except Exception as e:
        print(f"❌ ERRO inesperado: {e}")
        sys.exit(3)


if __name__ == "__main__":
    main()
