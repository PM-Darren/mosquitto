#!/bin/bash
DATE=$(date +%T)
# HOST=180.110.183.133
# HOST=skyworth2014.wicp.net
HOST=172.22.198.174
CURRENT_PATH=$(pwd)
while [[ 1 ]]; do
	mosquitto_pub -h $HOST -p 62000 -i zhaowei -t "zhaowei/time" -m current_time:$(date +%T) 
#	echo sndmsg:$(date +%T)
	sleep 5
done

