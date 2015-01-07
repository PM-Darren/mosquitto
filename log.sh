# /bin/bash

if [[ $# == 0 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog
elif [[ $# == 1 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep $1
elif [[ $# == 2 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep $1 | grep $2
elif [[ $# == 3 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep $1 | grep $2 | grep $3
elif [[ $# == 4 ]]; then
	echo "zhaowei" | sudo -S tail -f /var/log/syslog | grep $1 | grep $2 | grep $3 | grep $4
fi
