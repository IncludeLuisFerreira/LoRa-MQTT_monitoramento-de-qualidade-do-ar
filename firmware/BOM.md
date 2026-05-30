# Bill of Materials (BOM) - LoRaFieldMonitor (AirSense)

## Metadados
- **Autor:** Equipe de Desenvolvimento IoT
- **Data:** 30/05/2026
- **Versão:** 1.1

### Histórico de Revisões
| Versão | Data | Autor | Descrição da Alteração |
|--------|------|-------|------------------------|
| 1.0 | 30/05/2026 | Equipe de Desenvolvimento IoT | Emissão inicial do documento. |
| 1.1 | 30/05/2026 | Equipe de Desenvolvimento IoT | Correção dos sensores (DHT22 e ZP07) e atualização do resumo do projeto. |

## Resumo do Projeto
O LoRaFieldMonitor (AirSense) End Node é um dispositivo de monitoramento da qualidade do ar e condições ambientais de baixo consumo. O hardware utiliza um microcontrolador ESP32 para processamento, um rádio LoRa SX1276 para comunicação de longo alcance e sensores para medir umidade, temperatura e níveis de poluição. O sistema é alimentado por energia solar com bateria LiPo integrada, garantindo autonomia em campo.

## Microcontrolador
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 1 | 1 | U1 | SoC Wi-Fi & Bluetooth 4MB Flash | ESP32-WROOM-32E | Espressif Systems | SMD Module | DigiKey | — |

## Conectividade
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 2 | 1 | U2 | Módulo Transceptor LoRa 915MHz | RFM95W-915S2 | HopeRF | SMD Module | Mouser | Baseado no chip Semtech SX1276 |
| 3 | 1 | ANT1 | Antena Helicoidal 915MHz | ANT-916-CW-HWR-SMA | Linx Technologies | SMA | DigiKey | — |

## Sensores
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 4 | 1 | U3 | Sensor de Umidade e Temperatura de Alta Precisão | AM2302 (DHT22) | Aosong | 4-pin Package | Mouser | Conectado ao GPIO4 |
| 5 | 1 | U4 | Sensor de Qualidade do Ar (Poluição) | ZP07-MP503 | Winsen | Módulo | AliExpress | Saída digital de 2 bits (GPIO34, GPIO35) |

## Gerenciamento de Energia
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 6 | 1 | U5 | Gerenciador de Carga Solar Li-Ion | CN3065 | Consonance | SOP-8 | AliExpress/LCSC | Otimizado para painéis solares |
| 7 | 1 | U6 | Regulador LDO 3.3V 600mA | AP2112K-3.3TRG1 | Diodes Inc. | SOT-23-5 | DigiKey | Baixo ruído |
| 8 | 1 | D1 | Diodo Schottky de Proteção 1A | SS14 | ON Semiconductor | SMA (DO-214AC) | Mouser | — |
| 9 | 1 | BAT1 | Bateria LiPo 3.7V 2000mAh | LP103450 | PKCELL | Conector JST-PH | Adafruit | — |
| 10 | 1 | SOL1 | Painel Solar 6V 1W (110x60mm) | SZ-60-1W | Vários | Cabo com conector | Vários | — |

## Componentes Passivos
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 11 | 4 | C1, C2, C3, C4 | Capacitor Cerâmico 10uF 10V X5R | CL10A106KP8NNNC | Samsung | 0603 | DigiKey | — |
| 12 | 2 | C5, C6 | Capacitor Cerâmico 100nF 25V X7R | 06033C104KAT2A | AVX | 0603 | Mouser | — |
| 13 | 1 | R1 | Resistor 10k Ohm 1/10W 1% | RC0603FR-0710KL | Yageo | 0603 | DigiKey | Pull-up DHT22 |
| 14 | 1 | R2 | Resistor 2.2k Ohm 1/10W 1% | RC0603FR-072K2L | Yageo | 0603 | DigiKey | Programação de corrente CN3065 |

## Conectores e Diversos
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 15 | 2 | J1, J2 | Conector JST-PH 2-Pin Horizontal | S2B-PH-K-S(LF)(SN) | JST | SMD | DigiKey | — |
| 16 | 1 | PCB1 | Placa de Circuito Impresso AirSense | LFM-AS-V1.1 | Fabricação Própria | Custom | JLCPCB | FR4, 2 camadas |

## Resumo de custo estimado
- **Custo Total Estimado (1 unidade):** ~ $35.00 USD
- **Moeda de Referência:** USD (Dólar Americano)
- **Observações:** Preços baseados em distribuidores para quantidades unitárias em Maio de 2026.

## Notas
- **Requisitos de montagem:** Todos os passivos são SMD 0603.
- **Componentes críticos:** Os sensores DHT22 e ZP07 possuem prazos de entrega variáveis.
- **Conformidade:** Todos os componentes sugeridos são compatíveis com a diretiva RoHS.
