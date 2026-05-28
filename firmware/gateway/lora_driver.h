#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// Pinagem PK-LoRa
#define SCK_PIN    5
#define MISO_PIN  19
#define MOSI_PIN  27
#define NSS_PIN   18
#define RST_PIN   14
#define DIO0_PIN  26

#define FREQUENCY_IN_HZ 915E6
#define TX_POWER 17
#define SPREADING_FACTOR 7
#define SIGNAL_BANDWIDTH 125E3
#define CODING_RATE_DENOMINATOR 8

bool init_lora();
void lora_send(uint8_t* buffer, size_t size);
int lora_receive(uint8_t* buffer, size_t size, uint32_t timeout_ms);
int8_t get_last_rssi();
float get_last_snr();

#endif
