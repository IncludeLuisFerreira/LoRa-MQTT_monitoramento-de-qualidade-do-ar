#ifndef GATEWAY_CONFIG_H
#define GATEWAY_CONFIG_H

#include <Arduino.h>

const char* ssid = "Andrea Maria";
const char* password = "10042411";
const char* mqtt_server = "192.168.127.18";   // IP do broker MQTT
const int mqtt_port = 1883;

const char* topic_rx = "airsense/gw-001/rx";   // Uplink (gateway → broker)
const char* topic_tx = "airsense/gw-001/tx";   // Downlink (broker → gateway)
const char* topic_ack = "airsense/gw-001/tx/ack";
const char* topic_status = "airsense/gw-001/status";

#define GATEWAY_ID 100

#endif