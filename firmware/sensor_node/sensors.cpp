#include "sensors.h"

DHT dht(DHTPIN, DHTTYPE);

void init_sensors() {
    dht.begin();
    pinMode(ZP07_A_PIN, INPUT);
    pinMode(ZP07_B_PIN, INPUT);
}

float read_humidity() {
    float h = dht.readHumidity();
    if (isnan(h)) return 0.0;
    return h;
}

uint8_t read_pollution() {
    uint8_t a = digitalRead(ZP07_A_PIN);
    uint8_t b = digitalRead(ZP07_B_PIN);
    return (a << 1) | b;
}

uint8_t get_sensor_status() {
    uint8_t status = 0;
    float h = dht.readHumidity();
    if (!isnan(h)) status |= 0x01;
    // Assume ZP07 sempre presente
    status |= 0x02;
    return status;
}