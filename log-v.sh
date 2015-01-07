#! /bin/bash

if [[ $# == 0 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog
elif [[ $# == 1 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep -v $1
elif [[ $# == 2 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep -v $1 | grep -v $2
elif [[ $# == 3 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep -v $1 | grep -v $2 | grep -v $3
elif [[ $# == 4 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep -v $1 | grep -v $2 | grep -v $3 | grep -v $4
fi
