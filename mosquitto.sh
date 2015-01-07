#!/bin/bash
DATE=$(date +%T)
HOST=180.110.183.133
mosquitto_pub -h $HOST -p 61883 -t sensor -m current_time:$(date +%T)

