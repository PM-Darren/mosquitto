#! /bin/bash

PRO_PATH=$(pwd)
PRO_LOG=mosquitto.log
# testpro 为要守护的可执行程序，即保证它是一直运行的
PROGRAM="mosquitto"
CONFIG_FILE="/home/zhaowei/mosquitto/mosquitto.conf"

if [ ! -f $PRO_PATH/$PRO_LOG ]; then  
	touch $PRO_PATH/$PRO_LOG
fi  

date >> $PRO_PATH/$PRO_LOG

echo "mosquitto daemon process start" >> $PRO_PATH/$PRO_LOG

while true ; do

    sleep 10
    PRO_NOW=`ps aux | grep $CONFIG_FILE | grep -v grep | wc -l`


    if [ $PRO_NOW -lt 1 ]; then
		mosquitto -c $CONFIG_FILE -v -d

		date >> $PRO_PATH/$PRO_LOG
		echo "mosquitto start" >> $PRO_PATH/$PRO_LOG

    fi


    PRO_STAT=`ps aux|grep $CONFIG_FILE |grep T|grep -v grep|wc -l`

    if [ $PRO_STAT -gt 0 ] ; then

		echo "Zhaowei" | sudo -S killall -9 $PROGRAM
		sleep 2
		mosquitto -c $CONFIG_FILE -v -d

		date >> $PRO_PATH/$PRO_LOG
		echo "mosquitto start" >> $PRO_PATH/$PRO_LOG
    fi

done

date >> $PRO_PATH/$PRO_LOG

echo "mosquitto daemon process stop" >> $PRO_PATH/$PRO_LOG

exit 0