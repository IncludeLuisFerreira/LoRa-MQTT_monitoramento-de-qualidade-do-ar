#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Gerador de dados fake para teste do analise_dados.py
Gera arquivo JSON no diretório dados/ com registros de luminosidade
simulando coleta de sensor IoT.
"""

import json
import os
import random
from datetime import datetime, timedelta

DADOS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "dados")


def gerar_dados_fake(n_registros=20, intervalo_segundos=5, arquivo="dados_luminosidade.json"):
    """
    Gera dados fake de luminosidade com timestamps sequenciais.

    Args:
        n_registros: Quantidade de registros a gerar
        intervalo_segundos: Intervalo entre leituras (segundos)
        arquivo: Nome do arquivo de saída
    """
    os.makedirs(DADOS_DIR, exist_ok=True)

    # Base temporal: agora, retrocedendo para ter histórico
    agora = datetime.now()
    base = agora - timedelta(seconds=n_registros * intervalo_segundos)

    registros = []
    for i in range(n_registros):
        ts = base + timedelta(seconds=i * intervalo_segundos)
        # Simula variação de luminosidade: 800-2500, com tendência
        valor_base = 1200 + (i % 10) * 100
        ruido = random.randint(-200, 200)
        luminosidade = max(500, min(3000, valor_base + ruido))

        registros.append({
            "date": ts.strftime("%Y-%m-%d %H:%M:%S"),
            "luminosidade": luminosidade
        })

    caminho = os.path.join(DADOS_DIR, arquivo)
    with open(caminho, "w", encoding="utf-8") as f:
        json.dump(registros, f, indent=4, ensure_ascii=False)

    print(f"✅ Gerados {n_registros} registros em: {caminho}")
    print(f"   Período: {registros[0]['date']} → {registros[-1]['date']}")
    print(f"   Luminosidade: {registros[0]['luminosidade']} → {registros[-1]['luminosidade']}")


if __name__ == "__main__":
    # Gera 20 registros com intervalo de 5s = janela de ~100s (permitirá fatiamento de 30s-60s)
    gerar_dados_fake(n_registros=20, intervalo_segundos=5)
