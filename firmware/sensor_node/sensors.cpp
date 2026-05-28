#include "sensors.h"

DHT dht(DHTPIN, DHTTYPE);

void init_sensors() {
    dht.begin();
    pinMode(ZP07_PIN, INPUT);
}

float read_humidity() {
    float h = dht.readHumidity();
    if (isnan(h)) return 0.0;
    return h;
}

uint8_t read_pollution() {
    int val = analogRead(ZP07_PIN);
    // Mapeia 12-bit ADC (0-4095) para 0-255
    return (uint8_t)(val / 16);
}

uint8_t get_sensor_status() {
    uint8_t status = 0;
    float h = dht.readHumidity();
    if (!isnan(h)) status |= 0x01; // Bit 0: DHT22
    // ZP07 é analógico, difícil detectar "presença" sem HW específico, assume sempre ON se lido
    status |= 0x02; // Bit 1: ZP07
    return status;
}
