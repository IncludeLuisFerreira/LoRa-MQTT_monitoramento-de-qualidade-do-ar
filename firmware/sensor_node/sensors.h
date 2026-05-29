/**
 * ARQUIVO HEADER PARA A SIMPLIFICAÇÃO DE LEITURA NOS SENSORES DHT22 E O ZP07
 * 
 * Este arquivo contém apenas as declarações das funções, verifique o sensors.cpp
 * para a implementação.
 */
#ifndef SENSORS_H
#define SENSORS_H

/**
 *      BIBLIOTECAS IMPORTADAS
 */
#include <Arduino.h>
#include <DHT.h>

// CONFIGURAÇÕES DO DHT22
#define DHTPIN 4
#define DHTTYPE DHT22

#define ZP07_A_PIN   34  
#define ZP07_B_PIN   35 


// DECLARAÇÕES DAS FUNÇÕES

/**
 *  @brief Inicialização dos sensores
 */
void init_sensors();

/**
 *  @brief Lê a umidade do DHT22
 */
float read_humidity();

/**
 *  @brief Lê a poluição do sensor 
 */
uint8_t read_pollution();

/**
 *  @brief Verifica o status dos dois sensores
 */
uint8_t get_sensor_status();

#endif
