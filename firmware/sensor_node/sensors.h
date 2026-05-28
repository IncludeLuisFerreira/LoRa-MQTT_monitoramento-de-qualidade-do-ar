#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define ZP07_PIN 34

void init_sensors();
float read_humidity();
uint8_t read_pollution();
uint8_t get_sensor_status();

#endif
