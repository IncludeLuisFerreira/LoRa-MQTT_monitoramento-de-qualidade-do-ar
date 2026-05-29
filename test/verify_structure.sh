#!/bin/bash
set -e
echo "Verificando estrutura de pastas..."
ls firmware/sensor_node
ls firmware/gateway
ls border/api
ls border/templates
ls infra/mosquitto
ls infra/telegraf
ls infra/influxdb
ls infra/grafana
ls docs/PRD.md
echo "Estrutura OK!"
