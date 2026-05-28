#ifndef GATEWAY_CONFIG_H
#define GATEWAY_CONFIG_H

#include <Arduino.h>

const char* ssid = "NOME_DA_SUA_REDE";
const char* password = "SENHA_DA_SUA_REDE";
const char* mqtt_server = "localhost";
const int mqtt_port = 1883;

const char* topic_rx = "airsense/gw-001/rx";
const char* topic_tx = "airsense/gw-001/tx";
const char* topic_ack = "airsense/gw-001/tx/ack";
const char* topic_status = "airsense/gw-001/status";

#define GATEWAY_ID 100

#endif
