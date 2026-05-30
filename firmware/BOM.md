# Bill of Materials (BOM) - LoRaFieldMonitor

## Metadados
- **Autor:** Equipe de Desenvolvimento IoT
- **Data:** 30/05/2026
- **Versão:** 1.0

### Histórico de Revisões
| Versão | Data | Autor | Descrição da Alteração |
|--------|------|-------|------------------------|
| 1.0 | 30/05/2026 | Equipe de Desenvolvimento IoT | Emissão inicial do documento. |

## Resumo do Projeto
O LoRaFieldMonitor End Node é um dispositivo de monitoramento ambiental de baixo consumo (low-power), projetado para capturar dados de temperatura, umidade e pressão atmosférica em locais remotos. O hardware utiliza um microcontrolador ESP32 para processamento, um rádio LoRa SX1276 para comunicação de longo alcance e é alimentado por um sistema de energia solar com bateria LiPo integrada, garantindo autonomia operacional em campo.

## Microcontrolador
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 1 | 1 | U1 | SoC Wi-Fi & Bluetooth 4MB Flash | ESP32-WROOM-32E | Espressif Systems | SMD Module | DigiKey | — |

## Conectividade
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 2 | 1 | U2 | Módulo Transceptor LoRa 915MHz | RFM95W-915S2 | HopeRF | SMD Module | Mouser | Baseado no chip Semtech SX1276 |
| 3 | 1 | ANT1 | Antena Helicoidal 915MHz | ANT-916-CW-HWR-SMA | Linx Technologies | SMA | DigiKey | Incluir conector U.FL para SMA se necessário |

## Sensores
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 4 | 1 | U3 | Sensor Digital de Umidade, Pressão e Temperatura | BME280 | Bosch Sensortec | LGA-8 | Mouser | Interface I2C utilizada |

## Gerenciamento de Energia
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 5 | 1 | U4 | Gerenciador de Carga Solar Li-Ion | CN3065 | Consonance | SOP-8 | AliExpress/LCSC | Otimizado para painéis solares |
| 6 | 1 | U5 | Regulador LDO 3.3V 600mA | AP2112K-3.3TRG1 | Diodes Inc. | SOT-23-5 | DigiKey | Baixo ruído e baixa corrente de repouso |
| 7 | 1 | D1 | Diodo Schottky de Proteção 1A | SS14 | ON Semiconductor | SMA (DO-214AC) | Mouser | Proteção contra polaridade reversa do painel |
| 8 | 1 | BAT1 | Bateria LiPo 3.7V 2000mAh | LP103450 | PKCELL | Conector JST-PH | Adafruit | — |
| 9 | 1 | SOL1 | Painel Solar 6V 1W (110x60mm) | SZ-60-1W | Vários | Cabo com conector | Vários | Montagem externa |

## Componentes Passivos
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 10 | 4 | C1, C2, C3, C4 | Capacitor Cerâmico 10uF 10V X5R | CL10A106KP8NNNC | Samsung | 0603 | DigiKey | Desacoplamento de alimentação |
| 11 | 2 | C5, C6 | Capacitor Cerâmico 100nF 25V X7R | 06033C104KAT2A | AVX | 0603 | Mouser | Filtro de alta frequência |
| 12 | 3 | R1, R2, R3 | Resistor 10k Ohm 1/10W 1% | RC0603FR-0710KL | Yageo | 0603 | DigiKey | Pull-ups I2C e Reset |
| 13 | 1 | R4 | Resistor 2.2k Ohm 1/10W 1% | RC0603FR-072K2L | Yageo | 0603 | DigiKey | Programação de corrente CN3065 (~500mA) |
| 14 | 1 | L1 | LED Indicador de Carga (Verde) | APT1608SGC | Kingbright | 0603 | Mouser | — |

## Conectores e Diversos
| Item | Qtd. | Referência(s) no esquema | Descrição | MPN (Part Number) | Fabricante | Encapsulamento (Footprint) | Fornecedor | Observações |
|------|------|---------------------------|-----------|--------------------|------------|----------------------------|------------|-------------|
| 15 | 2 | J1, J2 | Conector JST-PH 2-Pin Horizontal | S2B-PH-K-S(LF)(SN) | JST | SMD | DigiKey | Entradas Bateria e Painel Solar |
| 16 | 1 | PCB1 | Placa de Circuito Impresso LoRaFieldMonitor | LFM-EN-V1.0 | Fabricação Própria | Custom | JLCPCB | FR4, 2 camadas, 1.6mm |

## Resumo de custo estimado
- **Custo Total Estimado (1 unidade):** ~ $32.50 USD
- **Moeda de Referência:** USD (Dólar Americano)
- **Observações:** Preços baseados em distribuidores (DigiKey/Mouser) para quantidades unitárias em Maio de 2026. O custo pode ser reduzido para aproximadamente $18.00 USD em lotes de 1000 unidades.

## Notas
- **Requisitos de montagem:** Todos os componentes passivos são SMD 0603, permitindo montagem compacta. Recomenda-se estêncil de 120 µm a 150 µm para aplicação de pasta de solda.
- **Componentes críticos:** O módulo LoRa (RFM95W) e o sensor BME280 possuem lead times que podem variar entre 8 a 12 semanas em distribuidores oficiais.
- **Conformidade:** Todos os componentes listados são compatíveis com a diretiva RoHS (Restriction of Hazardous Substances).
- **Proteção:** O circuito solar inclui proteção contra descarga reversa via diodo Schottky (D1).
